#include <libssh/libssh.h>
#include <libssh/server.h>
#include <stdlib.h>
#include <stdio.h>

int main(void){
  ssh_bind bind;
  ssh_session session;
  ssh_message message;
  ssh_channel chan=0;
  int r;
  

  bind = ssh_bind_new();
  session = ssh_new();
  
  if (bind == NULL || session == NULL){
    exit(-1);
  }
  
  if (ssh_bind_options_set(bind, SSH_BIND_OPTIONS_HOSTKEY, "/etc/ssh/ssh_host_rsa_key") != SSH_OK){
    fprintf(stderr, "Error setting host key: %s\n",
            ssh_get_error(bind));
    exit(-1);
  }
  
  if (ssh_bind_listen(bind) != SSH_OK){
    fprintf(stderr, "Error listening on bind: %s\n",
            ssh_get_error(bind));
    exit(-1);
  }

  r = ssh_bind_accept(bind,session);
  if(r == SSH_ERROR){
    printf("error accepting a connection : %s\n",
	   ssh_get_error(bind));
    exit(-1);
  }

  if (ssh_handle_key_exchange(session) != SSH_OK) {
    printf("error handling key exchange: %s\n", ssh_get_error(session));
    exit(-1);
  }

  do {
    message = ssh_message_get(session);
    if(message){
      switch(ssh_message_type(message)){
      case SSH_REQUEST_CHANNEL_OPEN:
	printf("got here 2\n");
	if(ssh_message_subtype(message) == SSH_CHANNEL_SESSION){
	  chan = ssh_message_channel_request_open_reply_accept(message);
	  break;
	}
      default:
	printf("%d\n",ssh_message_type(message));
	ssh_message_reply_default(message);
      }
      ssh_message_free(message);
    }
  } while(message && !chan);
  if(!chan){
    printf("Error opening channgel: %s\n",ssh_get_error(session));
    ssh_finalize();
    exit(-1);
  }
  
  ssh_disconnect(session);
  ssh_bind_free(bind);
  return 0;
}
