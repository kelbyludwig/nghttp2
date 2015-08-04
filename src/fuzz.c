#include <assert.h>
#include <stdio.h>
#include <unistd.h>

#include "nghttp2_helper.h"
#include "nghttp2_session.h"

/*
 * Custom Structs
 */
typedef struct {
    uint8_t buf[65535];
    size_t length;
} accumulator;

typedef struct {
    uint8_t data[8192];
    uint8_t *datamark;
    uint8_t *datalimit;
    size_t feedseq[8192];
    size_t seqidx;
} scripted_data_feed;

typedef struct {
    accumulator *acc;
    scripted_data_feed *df;
    int frame_recv_cb_called, invalid_frame_recv_cb_called;
    uint8_t recv_frame_type;
    int frame_send_cb_called;
    uint8_t sent_frame_type;
    int frame_not_send_cb_called;
    uint8_t not_sent_frame_type;
    int not_sent_error;
    int stream_close_cb_called;
    uint32_t stream_close_error_code;
    size_t data_source_length;
    int32_t stream_id;
    size_t block_count;
    int data_chunk_recv_cb_called;
    const nghttp2_frame *frame;
    size_t fixed_sendlen;
    int header_cb_called;
    int begin_headers_cb_called;
    nghttp2_nv nv;
    size_t data_chunk_len;
    size_t padlen;
    int begin_frame_cb_called;
} my_user_data;

/*
 * Fake Callbacks
 */
static ssize_t null_send_callback(nghttp2_session *session _U_,
                                      const uint8_t *data _U_, size_t len,
                                      int flags _U_, void *user_data _U_) {
    return len;
}

static int on_begin_frame_callback(nghttp2_session *session _U_,
                                   const nghttp2_frame_hd *hd _U_,
                                   void *user_data) {
    my_user_data *ud = (my_user_data *)user_data;
    ++ud->begin_frame_cb_called;
    return 0;
}

static int on_frame_recv_callback(nghttp2_session *session _U_,
                                  const nghttp2_frame *frame, void *user_data) {
    my_user_data *ud = (my_user_data *)user_data;
    ++ud->frame_recv_cb_called;
    ud->recv_frame_type = frame->hd.type;
    return 0;
}

/*
 * Fuzzing Functions
 */
void fuzz_frame(void) {
    nghttp2_session *session;
    nghttp2_session_callbacks callbacks;
    my_user_data user_data;

    uint8_t input[1033];

    // Create session
    memset(&callbacks, 0, sizeof(nghttp2_session_callbacks));
    callbacks.send_callback = null_send_callback;
    callbacks.on_frame_recv_callback = on_frame_recv_callback;
    callbacks.on_begin_frame_callback = on_begin_frame_callback;
    nghttp2_session_client_new(&session, &callbacks, &user_data);
    nghttp2_inbound_frame *iframe = &session->iframe;
    iframe->state = NGHTTP2_IB_READ_HEAD;

    // Read input
    memset(input, 0, 1033);
    ssize_t length = read(0, input, 1033);
    assert(length >= 9);

    // Send Frame
    user_data.data_chunk_recv_cb_called = 0;
    user_data.frame_recv_cb_called = 0;
    nghttp2_session_mem_recv(session, input, length);

    return;
}

int main(int argc, char **argv){
  fuzz_frame();
  return 0;
}
