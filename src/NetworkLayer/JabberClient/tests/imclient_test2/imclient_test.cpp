// imclient_test2.cpp : Defines the entry point for the console application.
//

extern "C" 

#include "IMClient.h"
#include <iostream>
#include <ctime>    // For time()
#include <ctime>    // For time()
#include "UnicodeConvert.h"
#include "SHA1.h"

#define MAX_USERNAME_LEN 1024

int	g_UidCounter = 0;

vector<std::string> input;
CRITICAL_SECTION	input_lock;
bool				bOnnline = false;
bool				bWriteLog = false;

void CryptUsername(const char* username, char szReport[MAX_USERNAME_LEN]);

DWORD WINAPI input_thread(LPVOID param)
{
	IMClient *client = reinterpret_cast<IMClient*>(param);
	
	while (!cin.eof()) {
		std::string line;
		std::getline(cin, line);
	
		EnterCriticalSection(&input_lock);
		input.push_back(line);
		LeaveCriticalSection(&input_lock);
	}

	return 0;
}
/*
*	emulate different inputs 
*/
void handle_input(std::string &input, IMClient *client)
{
	if(input[0] == '=') // >test1 will send message to user named test1
	{
		do
		{
			std::string to = "b099e04fec75b7700ff3f44d5d2931767c2f98cd";  //anatolyg@supportspace.com    
			//std::string to = "8fcf8d4750e6b30f6ed2c1ea6bd29b477c69c646"; //zeleno2008@gmail.com
			//std::string to = input.substr( 1, input.length()-1 );
			std::string subject = "stargate_new_request";
		
			char	str[1024] = {0};
			char	templ_str[1024] = 	
				"[{\
						\"submissionDate\":{\"month\":2,\"day\":4,\"year\":107,\"time\":1173359990111,\"seconds\":50,\"timezoneOffset\":-120,\"date\":8,\"hours\":15,\"minutes\":19},\
						\"problemDescription\":\"My cat!!\",\"supportRequestSubmissionMode\":\"DIRECT\",\"state\":\"NEW\",\"customer\":{\"accountNonExpired\":true,\"registrationDate\":null,\"accountNonLocked\":true,\"province\":null,\"firstName\":\"jhon\",\"birthdate\":null,\"authorities\":[],\"id\":0,\"nickName\":\"\",\"postcode\":\"\",\"confirmPassword\":\"\",\"username\":\"customer1\",\"jabberUsername\":\"\",\"city\":null,\"gender\":\"M\",\"password\":\"1234\",\"passwordHint\":\"\",\"street\":\"\",\"roles\":[],\"version\":0,\"lastName\":\"snow\",\"credentialsNonExpired\":true,\"country\":null,\"useNickname\":false,\"enabled\":true,\"fullName\":\"jhon snow44\",\"email\":\"\",\"phoneNumber\":\"\",\"unregistrationDate\":null,\"website\":\"\"},\
						\"id";

			g_UidCounter++;
			printf("UID=%d\n", g_UidCounter);
			sprintf(str,"%s\":%d}]", templ_str, g_UidCounter);

			std::string body(str);
			client->send_msg(to, body, subject);
			Sleep(4000);

			//std::string to2 = input.substr( 1, input.length()-1 );
			std::string subject2 = "stargate_request_notification";

			char	str2[1024] = {0};//CLOSED  PICKED
			char	templ_str2[1024] = "{\
				\"supportRequest\":\
				{\"workflowId\":2242,\
				\"submissionDate\":{\"month\":4,\"day\":1,\"year\":107,\"nanos\":0,\"time\":1179738380000,\"seconds\":20,\"timezoneOffset\":-180,\"date\":21,\"hours\":12,\"minutes\":6},\
				\"problemDescription\":\"I have pain in the ass\",\
				\"supportRequestSubmissionMode\":\"DIRECT\",\
				\"state\":\"CANCELED\",\
				\"id\":%d},\
				\"state\":\"CANCELED\",\
				\"lastChangedState\":{\"month\":4,\"day\":1,\"year\":107,\"time\":1179738434031,\"seconds\":14,\"timezoneOffset\":-180,\"date\":21,\"hours\":12,\"minutes\":7},\
				\"id\":%d}";

			sprintf(str2,templ_str2,g_UidCounter,g_UidCounter);
			printf("%s\n",str2);
			std::string body2(str2);

			if(g_UidCounter>0)
			{
				client->send_msg(to, body2, subject2);
				printf("CANCELED UID=%d\n", g_UidCounter);
				//g_UidCounter--; //don't change 
			}

			Sleep(0);

		}while(true);
	}

	if(input[0] == '-')// >test1 will send message to user named test1
	{ 
		std::string to = "b9a6bb8a7d1d9929e0a10b4ec7c4516d3f23b7e9" ;
		//std::string to = input.substr( 1, input.length()-1 );
		std::string subject = "stargate_delete_requests";

		char	str[1024] = {0};
		char	templ_str[1024] = 	
			"[{\"submissionDate\":{\"month\":2,\"day\":4,\"year\":107,\"time\":1173359990111,\"seconds\":50,\"timezoneOffset\":-120,\"date\":8,\"hours\":15,\"minutes\":19},\
			    \"problemDescription\":\"My cat ate my pc\",\"supportRequestSubmissionMode\":\"DIRECT\",\"state\":\"NEW\",\"customer\":{\"accountNonExpired\":true,\"registrationDate\":null,\"accountNonLocked\":true,\"province\":null,\"firstName\":\"jhon\",\"birthdate\":null,\"authorities\":[],\"id\":0,\"nickName\":\"\",\"postcode\":\"\",\"confirmPassword\":\"\",\"username\":\"customer1\",\"jabberUsername\":\"\",\"city\":null,\"gender\":\"M\",\"password\":\"1234\",\"passwordHint\":\"\",\"street\":\"\",\"roles\":[],\"version\":0,\"lastName\":\"snow\",\"credentialsNonExpired\":true,\"country\":null,\"useNickname\":false,\"enabled\":true,\"fullName\":\"jhon snow44\",\"email\":\"\",\"phoneNumber\":\"\",\"unregistrationDate\":null,\"website\":\"\"},\
				\"id";
		
		printf("UID=%d\n", g_UidCounter);
		sprintf(str,"%s\":%d}]", templ_str, g_UidCounter);
		if(g_UidCounter>0)
		{
			g_UidCounter--;
			std::string body(str);
			client->send_msg(to, body, subject);
		}
		return;
	}
///C:\Work\TestProjects\CallsManager\3_07_a\tests\imclient_test2_rc2 
	if(input[0] == '$') // >test1 will send message to user named test1
	{ 
		std::string to = "b9a6bb8a7d1d9929e0a10b4ec7c4516d3f23b7e9";
		std::string subject = "stargate_consult_notification";
		
		//std::string to = input.substr(1, input.length()-1);
	
		char	str[1024] = {0};
		char	templ_str[1024] = 	
		"{\"workflowId\":1213,\"displayedMessage\":\"\",\"originator\":{\"dispatcher\":false,\"province\":null,\"providingService\":true,\"languages\":[],\"id\":0,\"firstName\":\"supporter1\",\"nickName\":\"\",\"profissionalProfile\":{\"workplace\":\"\",\"position\":\"\",\"serviceDescription\":\"\",\"profileId\":0,\"supporterCertifications\":[]},\"blogUrl\":\"\",\"phones\":[],\"certified\":false,\"postcode\":\"\",\"username\":\"eran@domain.com\",\"aboutMe\":\"\",\"jabberUsername\":\"supporter2\",\"city\":null,\
		\"gender\":\"M\",\"street\":\"\",\"displayUserName\":\"supporter1 space\",\"roles\":[],\"presence\":\"offline\",\"lastName\":\"space\",\"photoUri\":\"\",\"country\":{\"countryName\":\
		\"UNKNOWN\",\"countryId\":0},\"useNickname\":false,\"fullName\":\"supporter1 space\",\
		\"state\":{\"PENDING\":\
		\"UNKNOWN\",\"stateCode\":\"Un\",\"stateId\":0},\"email\":\"\",\"hobbies\":\"\",\"website\":\"\"},\"state\":\
		\"PENDING\",\"lastChangedState\":{\"month\":5,\"day\":4,\"year\":107,\"time\":1183019580890,\"seconds\":0,\"timezoneOffset\":-180,\
		\"date\":28,\"hours\":11,\"minutes\":33},\"id";

		g_UidCounter++;
		printf("UID=%d\n", g_UidCounter);
		sprintf(str,"%s\":%d}", templ_str, g_UidCounter);

		std::string body(str);
		client->send_msg(to, body, subject);
	}
/*
	//test stargate_request_notification option PICKUP call
	if(input[0] == '#') // >test1 will send message to user named test1
	{ 
		std::string to = "b099e04fec75b7700ff3f44d5d2931767c2f98cd";
		//std::string to = input.substr( 1, input.length()-1 );
		std::string subject = "stargate_request_notification";

		char	str[1024] = {0};//CLOSED  PICKED
		char	templ_str[1024] = "{\
			\"supportRequest\":\
			{\"workflowId\":2242,\
			\"submissionDate\":{\"month\":4,\"day\":1,\"year\":107,\"nanos\":0,\"time\":1179738380000,\"seconds\":20,\"timezoneOffset\":-180,\"date\":21,\"hours\":12,\"minutes\":6},\
			\"problemDescription\":\"I have pain in the ass\",\
			\"supportRequestSubmissionMode\":\"DIRECT\",\
			\"state\":\"PENDING\",\
			\"id\":%d},\
			\"state\":\"PENDING\",\
			\"lastChangedState\":{\"month\":4,\"day\":1,\"year\":107,\"time\":1179738434031,\"seconds\":14,\"timezoneOffset\":-180,\"date\":21,\"hours\":12,\"minutes\":7},\
			\"id\":%d}";

		sprintf(str,templ_str,g_UidCounter,g_UidCounter);
		printf("%s\n",str);
		std::string body(str);

		//if(g_UidCounter>0)
		{
			//g_UidCounter--; don't change 
			client->send_msg(to, body, subject);
			printf("UID=%d\n", g_UidCounter);
		}

		return;
	}
*/
	if(input[0] == '+')		// >test1 will send message to user named test1
	{ 
		//std::string to = input.substr( 1, input.length()-1 );
		//std::string to = "b099e04fec75b7700ff3f44d5d2931767c2f98cd";  //anatolyg@supportspace.com  
		std::string to = "46ef98fb2a0d99145558a099f128cb048611d64e";  //papaj@supportspace.com
		//std::string to = "8fcf8d4750e6b30f6ed2c1ea6bd29b477c69c646"; //zeleno2008@gmail.com
		std::string subject = "stargate_new_request";
	
		char	str[1024] = {0};
		char	templ_str[1024] = 	
			"[{\
					\"submissionDate\":{\"month\":2,\"day\":4,\"year\":107,\"time\":1173359990111,\"seconds\":50,\"timezoneOffset\":-120,\"date\":8,\"hours\":15,\"minutes\":19},\
					\"problemDescription\":\"Shalom!!\",\"supportRequestSubmissionMode\":\"DIRECT\",\"state\":\"NEW\",\"customer\":{\"accountNonExpired\":true,\"registrationDate\":null,\"accountNonLocked\":true,\"province\":null,\"firstName\":\"jhon\",\"birthdate\":null,\"authorities\":[],\"id\":0,\"nickName\":\"\",\"postcode\":\"\",\"confirmPassword\":\"\",\"username\":\"customer1\",\"jabberUsername\":\"\",\"city\":null,\"gender\":\"M\",\"password\":\"1234\",\"passwordHint\":\"\",\"street\":\"\",\"roles\":[],\"version\":0,\"lastName\":\"snow\",\"credentialsNonExpired\":true,\"country\":null,\"useNickname\":false,\"enabled\":true,\"fullName\":\"jhon snow44\",\"email\":\"\",\"phoneNumber\":\"\",\"unregistrationDate\":null,\"website\":\"\"},\
					\"id";

		g_UidCounter++;
		printf("UID=%d\n", g_UidCounter);
		sprintf(str,"%s\":%d}]", templ_str, g_UidCounter);
/*
		std::wstring w_tmp_body(
			L"[{\
					\"submissionDate\":{\"month\":2,\"day\":4,\"year\":107,\"time\":1173359990111,\"seconds\":50,\"timezoneOffset\":-120,\"date\":8,\"hours\":15,\"minutes\":19},\
					\"problemDescription\":\"Моя кошка Русский!!!\",\"supportRequestSubmissionMode\":\"DIRECT\",\"state\":\"NEW\",\"customer\":{\"accountNonExpired\":true,\"registrationDate\":null,\"accountNonLocked\":true,\"province\":null,\"firstName\":\"jhon\",\"birthdate\":null,\"authorities\":[],\"id\":0,\"nickName\":\"\",\"postcode\":\"\",\"confirmPassword\":\"\",\"username\":\"customer1\",\"jabberUsername\":\"\",\"city\":null,\"gender\":\"M\",\"password\":\"1234\",\"passwordHint\":\"\",\"street\":\"\",\"roles\":[],\"version\":0,\"lastName\":\"snow\",\"credentialsNonExpired\":true,\"country\":null,\"useNickname\":false,\"enabled\":true,\"fullName\":\"jhon snow44\",\"email\":\"\",\"phoneNumber\":\"\",\"unregistrationDate\":null,\"website\":\"\"},\
					\"id\":10}]"); 

		std::string  s_tmp_body = ToUtf8FromUtf16(w_tmp_body);
		std::string body(str);
		client->send_msg(to, s_tmp_body, subject);
*/
		std::string body(str);
		client->send_msg(to, body, subject);
		return;
	}

	if(input[0] == '?')		// >test1 will send message "stargate_busy" to user named
	{ 
		//std::string to = input.substr( 1, input.length()-1 );
		//std::string to = "b099e04fec75b7700ff3f44d5d2931767c2f98cd";  //anatolyg@supportspace.com   
		std::string to = "e965c8f147757a5e216b341046649e72344b9266";  //papaj@supportspace.com
		//std::string to = "8fcf8d4750e6b30f6ed2c1ea6bd29b477c69c646"; //zeleno2008@gmail.com
		
		//std::string subject = "stargate_busy";
		//std::string subject = "stargate_busy_lock";
		//std::string subject = "stargate_online";
		//std::string subject = "stargate_online_4customer";
		std::string subject = "stargate_unread_messages";
/*					
		char	str[1024] = {0};
		char	templ_str[1024] = 	
			"[{\
					\"submissionDate\":{\"month\":2,\"day\":4,\"year\":107,\"time\":1173359990111,\"seconds\":50,\"timezoneOffset\":-120,\"date\":8,\"hours\":15,\"minutes\":19},\
					\"numberOfUnreadMessages\":\"You have 2 new messages.\",\"supportRequestSubmissionMode\":\"DIRECT\",\"state\":\"NEW\",\"customer\":{\"accountNonExpired\":true,\"registrationDate\":null,\"accountNonLocked\":true,\"province\":null,\"firstName\":\"jhon\",\"birthdate\":null,\"authorities\":[],\"id\":0,\"nickName\":\"\",\"postcode\":\"\",\"confirmPassword\":\"\",\"username\":\"customer1\",\"jabberUsername\":\"\",\"city\":null,\"gender\":\"M\",\"password\":\"1234\",\"passwordHint\":\"\",\"street\":\"\",\"roles\":[],\"version\":0,\"lastName\":\"snow\",\"credentialsNonExpired\":true,\"country\":null,\"useNickname\":false,\"enabled\":true,\"fullName\":\"jhon snow44\",\"email\":\"\",\"phoneNumber\":\"\",\"unregistrationDate\":null,\"website\":\"\"},\
					\"id";

		g_UidCounter++;
		printf("UID=%d\n", g_UidCounter);
		sprintf(str,"%s\":%d}]", templ_str, g_UidCounter);
*/

/*
		std::wstring w_tmp_body(
			L"[{\
					\"submissionDate\":{\"month\":2,\"day\":4,\"year\":107,\"time\":1173359990111,\"seconds\":50,\"timezoneOffset\":-120,\"date\":8,\"hours\":15,\"minutes\":19},\
					\"problemDescription\":\"Моя кошка Русский!!!\",\"supportRequestSubmissionMode\":\"DIRECT\",\"state\":\"NEW\",\"customer\":{\"accountNonExpired\":true,\"registrationDate\":null,\"accountNonLocked\":true,\"province\":null,\"firstName\":\"jhon\",\"birthdate\":null,\"authorities\":[],\"id\":0,\"nickName\":\"\",\"postcode\":\"\",\"confirmPassword\":\"\",\"username\":\"customer1\",\"jabberUsername\":\"\",\"city\":null,\"gender\":\"M\",\"password\":\"1234\",\"passwordHint\":\"\",\"street\":\"\",\"roles\":[],\"version\":0,\"lastName\":\"snow\",\"credentialsNonExpired\":true,\"country\":null,\"useNickname\":false,\"enabled\":true,\"fullName\":\"jhon snow44\",\"email\":\"\",\"phoneNumber\":\"\",\"unregistrationDate\":null,\"website\":\"\"},\
					\"id\":10}]"); 

		std::string  s_tmp_body = ToUtf8FromUtf16(w_tmp_body);
		std::string body(str);
		client->send_msg(to, s_tmp_body, subject);
*/
		char	str[1024] = "{numberOfUnreadMessages:3,supportMessageId:25}";

		std::string body(str);
		client->send_msg(to, body, subject);
		return;
	}

	if(input[0] == '*')// >test1 will send message to user named test1
	{ 
		std::string to = input.substr( 1, input.length()-1 );

		std::string subject_Flags = "Flags";
		std::string body_Flags = 
		   "[\
				{color:\"Blue\",	hex:\"#00557E\", label:\"Reviewed\"},\
				{color:\"Red\", 	hex:\"#FF0036\", label:\"Pending\"},\
				{color:\"Orange\",  hex:\"#FF6600\", label:\"Dont Know\"},\
				{color:\"Yellow\",  hex:\"#FFAE00\", label:\"To Do\"},\
				{color:\"Green\",	hex:\"#00aF2A\", label:\"Forget This Guy\"},\
				{color:\"Purple\",  hex:\"#F000FF\", label:\"Not a Problem\"},\
				{color:\"Black\",	hex:\"#000000\", label:\"Cant do\"}\
			]";

		std::string subject_Replies = "Replies";
		std::string body_Replies = 
		"[\
				{color:\"Red\",	  id:634653453,  label:\"Sorry, i am in...\"},\
				{color:\"Red\",   id:12344234234,label:\"Dont know how...\"},\
				{color:\"Red\",	  id:16642342,   label:\"I will get back...\"},\
				{color:\"Blue\",  id:43423423,   label:\"ASAP!!!\"},\
				{color:\"Blue\",  id:874325456,  label:\"You talkin to me?\"},\
				{color:\"Green\", id:230897323,  label:\"Yes... sure...\"},\
				{color:\"Green\", id:7475472243, label:\"You think so?\"},\
				{color:\"Yellow\",id:6388452324, label:\"I do it now.\"},\
				{color:\"Yellow\",id:732384672,  label:\"This is not a matter\"},\
				{color:\"Yellow\",id:9321847535, label:\"No problemo.\"}\
 		]";

		std::string subject_Snoozes = "Snoozes";
		std::string body_Snoozes =
			"[\
				{time:60,	label:\"1  Minute\"},\
				{time:300,	label:\"5  Minutes\"},\
				{time:900,	label:\"15 Minute\"},\
				{time:1800,	label:\"30 Minutes\"},\
				{time:3600,	label:\"1  Hour\"},\
				{time:7200,	label:\"2  Hours\"},\
				{time:10800,label:\"3  Hours\"}\
			]";

		std::string subject_Contacts = "Contacts";
		std::string body_Contacts =
			"[\
				{type:\"single\", color:\"Red\",	id:1634653453, name:\"Papa John\"},\
				{type:\"single\", color:\"Red\", 	id:212344234234, name:\"Vinst Gali\"},\
				{type:\"single\", color:\"Red\",	id:316642342, name:\"Yair Green\"},\
				{type:\"single\", color:\"Blue\",	id:443423423, name:\"Orit Stam\"},\
				{type:\"single\", color:\"Blue\",	id:5874325456, name:\"Steve Mark\"},\
				{type:\"single\", color:\"Green\",	id:6230897323, name:\"Shelly Rama\"},\
				{type:\"single\", color:\"Green\",	id:77475472243, name:\"Chris Isaac\"},\
				{type:\"single\", color:\"Green\",	id:87475472243, name:\"Alian Delon\"},\
				{type:\"single\", color:\"Yellow\", id:96388452324, name:\"Pierre Richard\"},\
				{type:\"group\", color:\"Red\",  	id:9634653453, name:\"My Friends\"},\
				{type:\"group\", color:\"Red\", 	id:812344234234, name:\"Wireless Group\"},\
				{type:\"group\", color:\"Red\", 	id:716642342, name:\"All\"},\
				{type:\"group\", color:\"Blue\",	id:643423423, name:\"Nana Group\"},\
				{type:\"group\", color:\"Blue\",	id:5874325456, name:\"Internet Experts\"},\
				{type:\"group\", color:\"Green\",	id:4230897323, name:\"MVPS\"},\
				{type:\"group\", color:\"Green\",	id:37475472243, name:\"Guru Net\"},\
				{type:\"group\", color:\"Yellow\",  id:27475472243, name:\"Admins\"},\
				{type:\"group\", color:\"Yellow\",  id:16388452324, name:\"House Callers\"}\
			]";
		
		client->send_msg(to, body_Flags, subject_Flags );
		client->send_msg(to, body_Replies, subject_Replies );
		client->send_msg(to, body_Snoozes, subject_Snoozes );
		client->send_msg(to, body_Replies, subject_Replies  );
		client->send_msg(to, body_Contacts, subject_Contacts );

		return;
	}
	
	if(input[0] == 'p')// 'p' - test update_status and itemAvailable and itemUnAvailable callbacks calls
	{ 
		bOnnline = !bOnnline;
		if( bOnnline ){
			client->update_status( PresenceUnavailable );
			printf( "user changed status to PresenceUnavailable\n");
		}
		else{
			client->update_status( PresenceAvailable );
			printf( "user changed status to PresenceAvailable\n");
		}
		return;
	}

	if(input[0] == 'l')// 'l' - test logging option 
	{ 
		bWriteLog = !bWriteLog;
		if( bWriteLog ){
			client->registerLogHandler();
			printf( "Write log enabled\n");
		}
		else{
			client->removeLogHandler();
			printf( "Write log disabled\n");
		}
		return;
	}

	//test new_version_availible option 
	if(input[0] == '!') // >test1 will send message to user named test1
	{ 
		std::string to = "8fcf8d4750e6b30f6ed2c1ea6bd29b477c69c646"; //zeleno2008@gmail.com
		//std::string to = input.substr( 1, input.length()-1 );
		std::string subject = "update_version";

		char	str[1024] = {0};
		char	templ_str[1024] = "{\"version\":\"4.0.44.6\"}";	

		std::string body(templ_str);
		client->send_msg(to, body, subject);
		return;
	}

	//test stargate_request_notification option PICKUP call
	if(input[0] == '#') // >test1 will send message to user named test1
	{ 
		std::string to = "b099e04fec75b7700ff3f44d5d2931767c2f98cd";  //anatolyg@supportspace.com    
		//std::string to = "8fcf8d4750e6b30f6ed2c1ea6bd29b477c69c646"; //zeleno2008@gmail.com
		//std::string to = input.substr( 1, input.length()-1 );
		//std::string to = input.substr( 1, input.length()-1 );
		std::string subject = "stargate_request_notification";

		char	str[1024] = {0};//CLOSED  PICKED
		char	templ_str[1024] = "{\
			\"supportRequest\":\
			{\"workflowId\":2242,\
			\"submissionDate\":{\"month\":4,\"day\":1,\"year\":107,\"nanos\":0,\"time\":1179738380000,\"seconds\":20,\"timezoneOffset\":-180,\"date\":21,\"hours\":12,\"minutes\":6},\
			\"problemDescription\":\"I have pain in the ass\",\
			\"supportRequestSubmissionMode\":\"DIRECT\",\
			\"state\":\"PICKED\",\
			\"id\":%d},\
			\"state\":\"PICKED\",\
			\"lastChangedState\":{\"month\":4,\"day\":1,\"year\":107,\"time\":1179738434031,\"seconds\":14,\"timezoneOffset\":-180,\"date\":21,\"hours\":12,\"minutes\":7},\
			\"id\":%d}";

		sprintf(str,templ_str,g_UidCounter,g_UidCounter);
		printf("%s\n",str);
		std::string body(str);

		if(g_UidCounter>0)
		{
			//g_UidCounter--; don't change 
			client->send_msg(to, body, subject);
			printf("UID=%d\n", g_UidCounter);
		}
		return;
	}


	//test stargate_request_notification option TIMEOUT call
	if(input[0] == 't') // >test1 will send message to user named test1
	{ 
		std::string to = input.substr( 1, input.length()-1 );
		std::string subject = "stargate_request_notification";

		char	str[1024] = {0};
		char	templ_str[1024] = "{\
			\"supportRequest\":\
			{\"workflowId\":2242,\
			\"submissionDate\":{\"month\":4,\"day\":1,\"year\":107,\"nanos\":0,\"time\":1179738380000,\"seconds\":20,\"timezoneOffset\":-180,\"date\":21,\"hours\":12,\"minutes\":6},\
			\"problemDescription\":\"I have pain in the ass\",\
			\"supportRequestSubmissionMode\":\"DIRECT\",\
			\"state\":\"CLOSED\",\
			\"id\":%d},\
			\"state\":\"TIMEOUT\",\
			\"lastChangedState\":{\"month\":4,\"day\":1,\"year\":107,\"time\":1179738434031,\"seconds\":14,\"timezoneOffset\":-180,\"date\":21,\"hours\":12,\"minutes\":7},\
			\"id\":%d}";

		sprintf(str,templ_str,g_UidCounter,g_UidCounter);
		printf("%s\n",str);
		std::string body(str);

		if(g_UidCounter>0)
		{
			g_UidCounter--;
			client->send_msg(to, body, subject);
			printf("UID=%d\n", g_UidCounter);
		}

		return;
	}
}

int main(int argc, char *argv[])
{
	InitializeCriticalSection(&input_lock);

	std::string username;
	std::string resource = "IMClient";
	std::string server;
	std::string password;
	std::string server_addr;
	std::string mesasage_subject;

	cout << "Username [su1@s2.com]\t\t: ";
	std::getline(cin, username);
	if (username.length() == 0)
		username = "su1@s2.com";
//
	char szReport[MAX_USERNAME_LEN] = { 0 };
	CryptUsername(username.c_str(), szReport);
	string crypt_username(szReport);
//

	cout << "Server [supportspace.com]\t: ";
	std::getline(cin, server);
	if (server.length() == 0)
		server = "supportspace.com";

	cout << "Password [1]\t\t\t: ";
	std::getline(cin, password);
	if (password.length()   == 0)
		password = "tomcat";

	cout << "Server Address [jabber.supportspace.com]\t: ";
	std::getline(cin, server_addr);
	if (server_addr.length() == 0)
		server_addr = "jabber.supportspace.com";
		
	IMClient client(crypt_username, resource, password, server, server_addr, NULL, true);

	// Start input thread
	DWORD input_thread_id;
	HANDLE input_thread_handle = CreateThread(NULL, 64*1024, input_thread, &client, 0, &input_thread_id);
	if (!input_thread_handle) {
		cout << "failed to create input thread." << endl;
		exit(0);
	}

	cout << "=== Connecting to server..." << endl;

	cout << "===================================================" << endl;
	cout << "===================================================" << endl;
	cout << "===================================================" << endl;
	cout << "To send update settings use: *<username> " << endl;
	cout << "To send new call        use: +<username> " << endl;
	cout << "To send delete calls    use: -<username> " << endl;
	cout << "===================================================" << endl;
	cout << "===================================================" << endl;
	cout << "===================================================" << endl;
	
	bool	bBlocked = false;

	client.connect(bBlocked);

	while (client.idle(1000) == ConnNoError) {
		EnterCriticalSection(&input_lock);
		while (input.size() > 0) {
			std::string line = input.back();
			input.pop_back();
			handle_input(line, &client);
		}
		LeaveCriticalSection(&input_lock);
	}
	cout << "=== Client terminated" << endl;

	return 0;
}

void CryptUsername(const char* username, char szReport[MAX_USERNAME_LEN])
{
	//
	//	4)	Crypt Supporter username with SHA1
	//
	char  hashBuf[SHA1Size + 1] = {0};

	CSHA1 m_crypt;

	m_crypt.MakeHash(
		(char*)username, 
		(int)strlen((char*)username), 
		hashBuf);


	m_crypt.ReportHash((unsigned __int8*)hashBuf, szReport, MAX_USERNAME_LEN);
}
