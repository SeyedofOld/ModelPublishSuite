#pragma once

// Server defines
//#define STORE_URL					"localhost"
#define STORE_URL					"51.254.82.69"
#define STORE_PORT					5617
//#define STORE_PORT					443
//#define STORE_SCHEME				"https"
#define STORE_SCHEME				"http"

#define MODEL_SERVICE_PATH			"/3dscan/"
#define MODEL_API_GET_INFO			"getinfo"
#define MODEL_API_GET				"get"
#define MODEL_API_GET_AD			"getad"

#define MODEL_CLIENT_ID_PCWIN		"pcwin"

#define MODEL_API_MAGIC				"tfzfepg"

// Database defines
#define MYSQL_SERVER		"tcp://127.0.0.1:3306"
#define MYSQL_USER			"root"
#define MYSQL_PASS			"1234"
#define STORE_DATABASE_NAME	"db_model_publish"

#define HMAC_SECRET_KEY		"Everything" // the light touches is our kingdom"

#define MODEL_FILE_EXTENSION	".3dscan"
#define AD_FILE_EXTENSION		".png"
#define MODEL_FILE_PATH			"Models//"
#define MODEL_AD_PATH			"Ads//"