/*********************************************************
*
* Description: Declarations for the authenticator module
* Author: Moshe Yifrah
*
*********************************************************/

#ifndef AUTHENTICATOR_H
#define AUTHENTICATOR_H

/* Implementation Functions ***********************************/

int AddUser(const char* user_name, const char* password);

/* -----------------------------------------------------------*/

int DeleteUser(const char* user_name, const char* password);

/* -----------------------------------------------------------*/

int Authenticate(const char* user_name, const char* password);

/* -----------------------------------------------------------*/
 
#endif /* AUTHENTICATOR_H */
