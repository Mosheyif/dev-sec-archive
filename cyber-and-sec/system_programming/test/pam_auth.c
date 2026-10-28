/*********************************************************
*
* Description: PAM authentication demo using misc_conv
*              conversation function
* Author: Moshe Yifrah
*
* Compile with: gcc -o pam_auth pam_auth.c -lpam -lpam_misc
*
* Run by: ./pam_auth
*
*********************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <termios.h>
#include <unistd.h>

#include <security/pam_appl.h>
#include <security/pam_misc.h>

#define MAX_BUFFER 64

void GetLineSafe(char *buffer, size_t size);

void GetPasswordHidden(char *buffer, size_t size);

int pam_conversation(int num_msg, const struct pam_message **msg, struct pam_response **resp, void *appdata_ptr) 
{
    int i;
    struct pam_response *reply;

    reply = (struct pam_response *)calloc(num_msg, sizeof(struct pam_response));
    if (reply == NULL) return PAM_BUF_ERR;

    for (i = 0; i < num_msg; i++) {
        if (msg[i]->msg_style == PAM_PROMPT_ECHO_OFF) {
            reply[i].resp = StrDup((char *)appdata_ptr);
            reply[i].resp_retcode = 0;
        }
    }

    *resp = reply; 
    return PAM_SUCCESS;
}

int AuthenticatePAM(const char* user_name, const char* password)
{
pam_handle_t *pamh = NULL;
int retval;
	
struct pam_conv conv;
conv.conv = pam_conversation;
conv.appdata_ptr = (void *)password;

retval = pam_start("my_auth_app", user_name, &conv, &pamh);

if (retval == PAM_SUCCESS) {
    retval = pam_authenticate(pamh, 0);
}

if (retval == PAM_SUCCESS) {
    retval = pam_acct_mgmt(pamh, 0);
}

pam_end(pamh, retval);

return (retval == PAM_SUCCESS ? 1 : 0);


}

int test_pam()
{
char user_name[MAX_BUFFER];
char password[MAX_BUFFER];

printf("--- Linux PAM Authentication System ---\n");
printf("Enter system username: ");
GetLineSafe(user_name, MAX_BUFFER);
printf("Enter system password: ");
GetPasswordHidden(password, MAX_BUFFER);

if (AuthenticatePAM(user_name, password))
    printf("[+] Authentication successful! Welcome, %s.\n", user_name);
else
    printf("[-] Authentication failed.\n");

return 0;


}
