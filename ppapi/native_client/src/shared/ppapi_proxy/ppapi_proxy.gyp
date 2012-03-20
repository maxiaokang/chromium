# Copyright (c) 2012 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

{
  'variables': {
    'chromium_code': 1,  # Use higher warning level.
  },
  'includes': [
    '../../../../../native_client/build/common.gypi',
  ],
  'target_defaults': {
    'conditions': [
      ['OS=="linux"', {
        'cflags!': [
          '-Wno-unused-parameter', # be a bit stricter to match NaCl flags.
        ],
      }],
      ['OS=="mac"', {
        'cflags!': [
          '-Wno-unused-parameter', # be a bit stricter to match NaCl flags.
        ],
      }],
    ],
  },
  'targets': [
    {
      'target_name': 'nacl_ppapi_browser',
      'type': 'static_library',
      'sources': [
        'browser_callback.cc',
        'browser_globals.cc',
        'browser_nacl_file_rpc_server.cc',
        'browser_ppb_audio_config_rpc_server.cc',
        'browser_ppb_audio_rpc_server.cc',
        'browser_ppb_core_rpc_server.cc',
        'browser_ppb_cursor_control_rpc_server.cc',
        'browser_ppb_file_io_rpc_server.cc',
        'browser_ppb_file_ref_rpc_server.cc',
        'browser_ppb_file_system_rpc_server.cc',
        'browser_ppb_find_rpc_server.cc',
        'browser_ppb_font_rpc_server.cc',
        'browser_ppb_fullscreen_rpc_server.cc',
        'browser_ppb_gamepad_rpc_server.cc',
        'browser_ppb_graphics_2d_rpc_server.cc',
        'browser_ppb_graphics_3d_rpc_server.cc',
        'browser_ppb_host_resolver_private_rpc_server.cc',
        'browser_ppb_image_data_rpc_server.cc',
        'browser_ppb_input_event_rpc_server.cc',
        'browser_ppb_instance_rpc_server.cc',
        'browser_ppb_messaging_rpc_server.cc',
        'browser_ppb_mouse_lock_rpc_server.cc',
        'browser_ppb_net_address_private_rpc_server.cc',
        'browser_ppb_pdf_rpc_server.cc',
        'browser_ppb_rpc_server.cc',
        'browser_ppb_scrollbar_rpc_server.cc',
        'browser_ppb_tcp_server_socket_private_rpc_server.cc',
        'browser_ppb_tcp_socket_private_rpc_server.cc',
        'browser_ppb_testing_rpc_server.cc',
        'browser_ppb_udp_socket_private_rpc_server.cc',
        'browser_ppb_url_loader_rpc_server.cc',
        'browser_ppb_url_request_info_rpc_server.cc',
        'browser_ppb_url_response_info_rpc_server.cc',
        'browser_ppb_websocket_rpc_server.cc',
        'browser_ppb_widget_rpc_server.cc',
        'browser_ppb_zoom_rpc_server.cc',
        'browser_ppp_find.cc',
        'browser_ppp_input_event.cc',
        'browser_ppp_instance.cc',
        'browser_ppp_messaging.cc',
        'browser_ppp_mouse_lock.cc',
        'browser_ppp_printing.cc',
        'browser_ppp_scrollbar.cc',
        'browser_ppp_selection.cc',
        'browser_ppp_widget.cc',
        'browser_ppp_zoom.cc',
        'browser_ppp.cc',
        'browser_upcall.cc',
        'input_event_data.cc',
        'object_serialize.cc',
        'ppp_instance_combined.cc',
        'utility.cc',
        'view_data.cc',
        # Autogerated files
        'ppp_rpc_client.cc',
        'ppb_rpc_server.cc',
        'upcall_server.cc',
      ],
      'include_dirs': [
        '<(DEPTH)/ppapi/native_client/src/shared/ppapi_proxy/trusted',
        '<(DEPTH)/ppapi',
      ],
      'dependencies': [
        '<(DEPTH)/ppapi/ppapi.gyp:ppapi_c',
      ],
    },
  ],
}
