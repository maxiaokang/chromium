// Copyright 2008, Google Inc.
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are
// met:
//
//    * Redistributions of source code must retain the above copyright
// notice, this list of conditions and the following disclaimer.
//    * Redistributions in binary form must reproduce the above
// copyright notice, this list of conditions and the following disclaimer
// in the documentation and/or other materials provided with the
// distribution.
//    * Neither the name of Google Inc. nor the names of its
// contributors may be used to endorse or promote products derived from
// this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
// "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
// LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
// A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
// OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
// SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
// LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
// DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
// THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
// (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

#include "config.h"

#pragma warning(push, 0)
#include "ContextMenu.h"
#include "Document.h"
#include "DocumentLoader.h"
#include "Editor.h"
#include "FrameLoader.h"
#include "FrameView.h"
#include "HitTestResult.h"
#include "KURL.h"
#include "Widget.h"
#pragma warning(pop)
#undef LOG

#include "webkit/glue/context_menu_client_impl.h"

#include "base/string_util.h"
#include "webkit/glue/context_node_types.h"
#include "webkit/glue/glue_util.h"
#include "webkit/glue/webdocumentloader_impl.h"
#include "webkit/glue/webview_impl.h"

#include "base/word_iterator.h"

// Helper function to determine whether text is a single word or a sentence.
static bool IsASingleWord(const std::wstring& text) {
  WordIterator iter(text, WordIterator::BREAK_WORD);
  int word_count = 0;
  if (!iter.Init()) return false;
  while (iter.Advance()) {
    if (iter.IsWord()) {
      word_count++;
      if (word_count > 1) // More than one word.
        return false;
    }
  }
  
  // Check for 0 words.
  if (!word_count)
    return false;
  
  // Has a single word.
  return true;
}

// Helper function to get misspelled word on which context menu 
// is to be evolked. This function also sets the word on which context menu
// has been evoked to be the selected word, as required.
static std::wstring GetMisspelledWord(WebCore::ContextMenu* default_menu,
                                      WebCore::Frame* selected_frame) {
  std::wstring misspelled_word_string;

  // First select from selectedText to check for multiple word selection.
  misspelled_word_string = CollapseWhitespace(
      webkit_glue::StringToStdWString(selected_frame->selectedText()),
      false);

  // Don't provide suggestions for multiple words.
  if (!misspelled_word_string.empty() && 
      !IsASingleWord(misspelled_word_string))
    return L"";

  // Expand around the click to see if we clicked a word.
  WebCore::Selection selection;
  WebCore::VisiblePosition pos(default_menu->hitTestResult().innerNode()->
      renderer()->positionForPoint(default_menu->hitTestResult().
                                   localPoint()));

  if (pos.isNotNull()) {
    selection = WebCore::Selection(pos);
    selection.expandUsingGranularity(WebCore::WordGranularity);
  }
      
  if (selection.isRange()) { 
    selected_frame->setSelectionGranularity(WebCore::WordGranularity);
  }
        
  if (selected_frame->shouldChangeSelection(selection))
    selected_frame->selectionController()->setSelection(selection);
       
  misspelled_word_string = CollapseWhitespace(
      webkit_glue::StringToStdWString(selected_frame->selectedText()),                  
                                      false);
  
  return misspelled_word_string;
}

ContextMenuClientImpl::~ContextMenuClientImpl() {
}

void ContextMenuClientImpl::contextMenuDestroyed() {
  delete this;
}

// Figure out the URL of a page or subframe. Returns |page_type| as the type,
// which indicates page or subframe, or ContextNode::NONE if the URL could not
// be determined for some reason.
static ContextNode::Type GetTypeAndURLFromFrame(WebCore::Frame* frame,
                                                GURL* url,
                                                ContextNode::Type page_type) {
  ContextNode::Type type = ContextNode::NONE;
  if (frame) {
    WebCore::DocumentLoader* dl = frame->loader()->documentLoader();
    if (dl) {
      WebDataSource* ds = static_cast<WebDocumentLoaderImpl*>(dl)->
          GetDataSource();
      if (ds) {
        type = page_type;
        *url = ds->HasUnreachableURL() ? ds->GetUnreachableURL()
                                       : ds->GetRequest().GetURL();
      }
    }
  }
  return type;
}

WebCore::PlatformMenuDescription
    ContextMenuClientImpl::getCustomMenuFromDefaultItems(
        WebCore::ContextMenu* default_menu) {
  // Displaying the context menu in this function is a big hack as we don't
  // have context, i.e. whether this is being invoked via a script or in 
  // response to user input (Mouse event WM_RBUTTONDOWN,
  // Keyboard events KeyVK_APPS, Shift+F10). Check if this is being invoked
  // in response to the above input events before popping up the context menu.
  if (!webview_->context_menu_allowed())
    return NULL;

  WebCore::HitTestResult r = default_menu->hitTestResult();
  WebCore::Frame* selected_frame = r.innerNonSharedNode()->document()->frame();

  WebCore::IntPoint menu_point =
      selected_frame->view()->contentsToWindow(r.point());

  ContextNode::Type type = ContextNode::NONE;

  // Links, Images and Image-Links take preference over all else.
  WebCore::KURL link_url = r.absoluteLinkURL();
  std::wstring link_url_string;
  if (!link_url.isEmpty()) {
    type = ContextNode::LINK;
  }
  WebCore::KURL image_url = r.absoluteImageURL();
  std::wstring image_url_string;
  if (!image_url.isEmpty()) {
    type = ContextNode::IMAGE;
  }
  if (!image_url.isEmpty() && !link_url.isEmpty())
    type = ContextNode::IMAGE_LINK;

  // If it's not a link, an image or an image link, show a selection menu or a
  // more generic page menu.
  std::wstring selection_text_string;
  std::wstring misspelled_word_string;
  GURL frame_url;
  GURL page_url;
  
  std::wstring frame_encoding;
  // Send the frame and page URLs in any case.
  ContextNode::Type frame_type = ContextNode::NONE;
  ContextNode::Type page_type = 
      GetTypeAndURLFromFrame(webview_->main_frame()->frame(),
                             &page_url,
                             ContextNode::PAGE);
  if (selected_frame != webview_->main_frame()->frame()) {
    frame_type = GetTypeAndURLFromFrame(selected_frame,
                                        &frame_url,
                                        ContextNode::FRAME);
    frame_encoding = webkit_glue::StringToStdWString(
        selected_frame->loader()->encoding());
  }
  
  if (type == ContextNode::NONE) {
    if (r.isContentEditable()) {
      type = ContextNode::EDITABLE;
      if (webview_->FocusedFrameNeedsSpellchecking()) {
        misspelled_word_string = GetMisspelledWord(default_menu, 
                                                   selected_frame);
      }
    } else if (r.isSelected()) {
      type = ContextNode::SELECTION;
      selection_text_string =
          CollapseWhitespace(
              webkit_glue::StringToStdWString(selected_frame->selectedText()),
              false);
    } else if (selected_frame != webview_->main_frame()->frame()) {
      type = frame_type;
    } else {
      type = page_type;
    }
  }

  int edit_flags = ContextNode::CAN_DO_NONE;
  if (webview_->GetFocusedWebCoreFrame()->editor()->canUndo())
    edit_flags |= ContextNode::CAN_UNDO;
  if (webview_->GetFocusedWebCoreFrame()->editor()->canRedo())
    edit_flags |= ContextNode::CAN_REDO;
  if (webview_->GetFocusedWebCoreFrame()->editor()->canCut())
    edit_flags |= ContextNode::CAN_CUT;
  if (webview_->GetFocusedWebCoreFrame()->editor()->canCopy())
    edit_flags |= ContextNode::CAN_COPY;
  if (webview_->GetFocusedWebCoreFrame()->editor()->canPaste())
    edit_flags |= ContextNode::CAN_PASTE;
  if (webview_->GetFocusedWebCoreFrame()->editor()->canDelete())
    edit_flags |= ContextNode::CAN_DELETE;
  // We can always select all...
  edit_flags |= ContextNode::CAN_SELECT_ALL;

  WebViewDelegate* d = webview_->delegate();
  if (d) {
    d->ShowContextMenu(webview_,
                       type,
                       menu_point.x(),
                       menu_point.y(),
                       webkit_glue::KURLToGURL(link_url),
                       webkit_glue::KURLToGURL(image_url),
                       page_url,
                       frame_url,
                       selection_text_string,
                       misspelled_word_string,
                       edit_flags,
                       frame_encoding);
  }
  return NULL;
}

void ContextMenuClientImpl::contextMenuItemSelected(
    WebCore::ContextMenuItem*, const WebCore::ContextMenu*) {
}

void ContextMenuClientImpl::downloadURL(const WebCore::KURL&) {
}

void ContextMenuClientImpl::copyImageToClipboard(const WebCore::HitTestResult&) {
}

void ContextMenuClientImpl::searchWithGoogle(const WebCore::Frame*) {
}

void ContextMenuClientImpl::lookUpInDictionary(WebCore::Frame*) {
}

void ContextMenuClientImpl::speak(const WebCore::String&) {
}

void ContextMenuClientImpl::stopSpeaking() {
}

bool ContextMenuClientImpl::shouldIncludeInspectElementItem() {
    return false;  // TODO(jackson): Eventually include the inspector context menu item
}
