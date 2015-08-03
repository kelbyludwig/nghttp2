#include <stdio.h>
#include "nghttp2_session.h"

void fuzz_frame(int argc, void *argv) {
    nghttp2_session *session;
    nghttp2_session_callbacks callbacks;
    my_users_data user_data;
    nghttp2_frame_hd hd;
    char *buf[2048]; 

    memset(&callbacks, 0, sizeof(nghttp2_session_callbacks));
    callbacks.send_callback = null_send_callback;
    callbacks.on_frame_recv_callback = on_frame_recv_callback;
    callbacks.on_begin_frame_callback = on_begin_frame_callback;

    nghttp2_session_client_new(&session, &callbacks, &user_data);

    memset(buf, 0, 2048);
    input_size = read(0, buf, 2048);

    hd.length = input_size;
    hd.type = NGHTTP2_DATA;
    hd.flags = NGHTTP2_FLAG_NONE;
    hd.stream_id = 1;
    nghttp2_frame_pack_frame_hd(buf, &hd);

    user_data.data_chunk_recv_cb_called = 0;
    user_data.frame_recv_cb_called = 0;
    rv = nghttp2_session_mem_recv(session, buf, NGHTTP2_FRAME_HDLEN + input_size);

    return 0;
}
