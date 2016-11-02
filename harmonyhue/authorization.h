#ifndef AUTHORIZATION_H
#define AUTHORIZATION_H

#include <string>

#include "hparser.h"
#include "hstates.h"

HARMONY::ERROR authorizeWithAuthorizationToken(); // on success return a non-empty session token

HARMONY::ERROR retriveAuthorizationTokenFromLogitech(); // on success return a non-empty authorization token

HARMONY::ERROR loginWithSessionToken(); // on success return a valid parser-object

HARMONY::ERROR connectToHubAndParse(); // use all functions above and return a valid parser-object on success

/* usage
 * 
 * std::string sessionToken = authorizeWithAuthorizationToken();
 * if (sessionToken.empty()) {
	* std::string authorizationToken = retriveAuthorizationTokenFromLogitech();
	* if (!authorizationToken.empty()) {
		* sessionToken = int authorizeWithAuthorizationToken();
		* if (sessionToken.empty()) {
			* exit(1); // error session token not retrived
		* }
	* } else {
		* exit(1); // error could not retrive authorization token
	* }
 * HParser* parser = loginWithSessionToken()
 * if (parser == 0) {
	* exit(1); // error could not login with session int authorizeWithAuthorizationToken()
 * }
 * 
 * go on ... and parse



	if (tryAuthorizationToken()) {
		retriveSessionToken(); // close connection to hub
	} else {
		if (retriveAuthorizationTokenFromLogitech()) {
			if (tryAuthorizationToken()) {
				if(!retriveSessionToken()) { // close connection to hub
					// Error session token not retrived
					exit(1);
				}
			} else {
				// Authorization token not valid
				exit(1);
			}
		} else {
			// no authorization token from logitech
			exit(1);
		}
	}
	if (!loginWithSessionToken()) {
		// error login with session token
		exit(1);
	}
*/

#endif
