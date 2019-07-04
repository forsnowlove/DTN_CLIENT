#include "stdio.h"
#include "stdlib.h"
#include "bluetooth/bluetooth.h"
#include "bluetooth/sdp.h"
#include "bluetooth/sdp_lib.h"
#include <unistd.h>
#include <sys/socket.h>

int main ( int argc , char * * argv )
{
    //uint32_t svc_uuid_int [] = {0x00,0x00,0x11,0x01,0x00,0x00,0x10,0x00,0x80,
    //                            0x00,0x00,0x80,0x5F,0x9B,0x34,0xFB};
    uint32_t svc_uuid_int[] = {0,0,0,0xABCD};
    int status;
    bdaddr_t target ;
    uuid_t svc_uuid ;
    sdp_list_t * response_list , * search_list , * attrid_list ;
    sdp_session_t * session = 0 ;
    uint32_t range = 0x0000ffff ;
    uint8_t port = 0 ;
    str2ba("00:27:13:C9:42:6E", &target ) ;
    //str2ba("14:9D:09:E3:FF:A9", &target ) ;
    // connect to the SDP server running on the remote machine
    session = sdp_connect( BDADDR_ANY , &target , 0 ) ;
    //int res = write(session->sock,"hello",6);
    //printf("res = %d\n",res);
    if(session == NULL)
    {
        printf("connect failed!\n");
        exit(-1);
    }
    sdp_uuid128_create ( &svc_uuid , &svc_uuid_int ) ;
    search_list = sdp_list_append( 0 , &svc_uuid ) ;
    attrid_list = sdp_list_append( 0 , &range ) ;
// get a list of service records that have UUID 0xabcd
    response_list = NULL ;
    status = sdp_service_search_attr_req( session , search_list ,
    SDP_ATTR_REQ_RANGE , attrid_list , &response_list ) ;

    if ( status == 0 ) {
        printf("111\n");
        sdp_list_t * proto_list = NULL ;
        sdp_list_t * r = response_list ;
// go through each of the service records
    for ( ; r ; r = r->next ) {
        sdp_record_t * rec = ( sdp_record_t * ) r->data ;
    // get a list of the protocol sequences
        if ( sdp_get_access_protos( rec , &proto_list ) == 0 ) {
            // get the rfcommge port number
            port = sdp_get_proto_port( proto_list , RFCOMM_UUID ) ;
            sdp_list_free( proto_list , 0 ) ;
        }
        sdp_record_free( rec ) ;
    }
    }
    sdp_list_free( response_list , 0 ) ;
    sdp_list_free( search_list , 0 ) ;
    sdp_list_free( attrid_list , 0 ) ;
    sdp_close ( session ) ;
    if ( port != 0 ) {
        printf ( "found service running on RFCOMM port %d\n " , port ) ;
    }
    return 0 ;
}
