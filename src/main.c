#include <libssh/libssh.h>
#include <libssh/server.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

static int auth_password(const char *user, const char *password){
    if(strcmp(user,""))
        return 0;
    if(strcmp(password,"lala"))
        return 0;
    return 1; // authenticated
}

int main(void){
  ssh_bind bind;
  ssh_session session;
  ssh_message message;
  ssh_channel chan=0;

  char buf[2048];
  int auth = 0;
  int shell = 0;
  int r, i;

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
    message=ssh_message_get(session);
    if(!message)
      break;
    switch(ssh_message_type(message)){
    case SSH_REQUEST_AUTH:
      switch(ssh_message_subtype(message)){
      case SSH_AUTH_METHOD_NONE:
        ssh_message_auth_reply_success(message,0);
	auth = 1;
	printf("User %s connected\n",
	       ssh_message_auth_user(message));
      default:
	ssh_message_auth_set_methods(message,SSH_AUTH_METHOD_NONE);
	ssh_message_reply_default(message);
	break;
      }
      break;
    default:
      ssh_message_reply_default(message);
    }
    ssh_message_free(message);
  } while (!auth);

  if(!auth){
    printf("auth error: %s\n",ssh_get_error(session));
    ssh_disconnect(session);
    return 1;
  }
  
  do {
    message = ssh_message_get(session);
    if(message){
      switch(ssh_message_type(message)){
      case SSH_REQUEST_CHANNEL_OPEN:
	if(ssh_message_subtype(message) == SSH_CHANNEL_SESSION){
	  chan = ssh_message_channel_request_open_reply_accept(message);
	  break;
	}
      default:
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

  do {
    message=ssh_message_get(session);
    if(message && ssh_message_type(message)==SSH_REQUEST_CHANNEL &&
       ssh_message_subtype(message)==SSH_CHANNEL_REQUEST_SHELL){
      shell=1;
      ssh_message_channel_request_reply_success(message);
      break;
    }
    if(!shell){
      ssh_message_reply_default(message);
    }
    ssh_message_free(message);
  } while (message && !shell);
  
  if(!shell){
    printf("error : %s\n",ssh_get_error(session));
    return 1;
  }

  do{
    i=ssh_channel_read(chan,buf, 2048, 0);
    if(i>0) {
      ssh_channel_write(chan, buf, i);
      if (write(1,buf,i) < 0) {
	printf("error writing to buffer\n");
	return 1;
      }
    }
  } while (i>0);
    
  ssh_disconnect(session);
  ssh_bind_free(bind);
  ssh_finalize();
  return 0;
}
