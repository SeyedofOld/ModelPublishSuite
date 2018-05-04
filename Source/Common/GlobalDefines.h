#pragma once

// Server defines
//#define STORE_URL					"localhost"
//#define MODEL_SERVICE_SERVER					"51.254.82.69"
//#define MODEL_SERVICE_SERVER					"18.220.78.255"
//#define MODEL_SERVICE_SERVER					"18.188.186.152"
//#define MODEL_SERVICE_SERVER					"94.130.249.203"
#define MODEL_SERVICE_SERVER					"127.0.0.1"
//#define MODEL_SERVICE_PORT						5617
//#define STORE_PORT					443
//#define STORE_SCHEME				"https"
#define MODEL_SERVICE_SCHEME				"http"

#define MODEL_SERVICE_PATH			"/3dscan/"
#define MODEL_API_GET_INFO			"getinfo"
#define MODEL_API_GET				"get"
#define MODEL_API_GET_AD			"getad"
#define MODEL_API_UPLOAD_MODEL		"upload_model"
#define MODEL_API_UPLOAD_AD			"upload_ad"
#define MODEL_API_CREATE_SUBSCRIPTION	"create_subs"

#define MODEL_CLIENT_ID_PCWIN		"pcwin"

#define MODEL_API_MAGIC				"tfzfepg"

// Database defines
#define MYSQL_SERVER				"tcp://127.0.0.1:3306"
#define MYSQL_USER					"root"
#define MYSQL_PASS					"1234"
#define SERVICE_DATABASE_NAME		"db_model_publish"

#define HMAC_SECRET_KEY		"Everything" // the light touches is our kingdom"

#define MODEL_FILE_EXTENSION	".3dscan"
#define AD_FILE_EXTENSION		".png"
#define MODEL_FILE_PATH			"Models//"
#define AD_FILE_PATH			"Ads//"

#define MS_ERROR_OK							0	
#define MS_ERROR_SUBSCRIPTION_NOT_FOUND		1
#define MS_ERROR_MODEL_NOT_FOUND			2
#define MS_ERROR_FILE_ADDRESS_NOT_FOUND		3
#define MS_ERROR_COULD_NOT_OPEN_FILE		4
#define MS_ERROR_AD_NOT_FOUND				5
#define MS_ERROR_UNKNOWN_CLIENT				6
#define MS_ERROR_UNKNOWN_METHOD				7
#define MS_ERROR_DB							8
#define MS_ERROR_INVALID_USER_PASS			9

#define MS_ERROR_UNKNOWN					100
