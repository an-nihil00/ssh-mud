#include <libssh/libssh.h>
#include <libssh/server.h>
#include <stdlib.h>
#include <stdio.h>

int main(void){
  ssh_bind bind;
  ssh_session session;
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

  ssh_disconnect(session);
  ssh_bind_free(bind);
  return 0;
}
