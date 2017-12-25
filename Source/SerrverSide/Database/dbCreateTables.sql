-- ------------------------------------------------------------------------------------
--                                                                                    -
--   Copyright(C) 2017 Ali Seyedof	(seyedof@gmail.com)								  -
--                                                                                    -
-- ------------------------------------------------------------------------------------

-- --------------------------------------------
-- Create tables
-- --------------------------------------------

-- Change the database name to match your own pre-created database
USE db_model_publish ;


-- This table holds file path
CREATE TABLE tbl_file_address
	(
		id int NOT NULL PRIMARY KEY AUTO_INCREMENT, -- id for file
		FilePathName varchar(128),					-- File path name on disk
		Size int NOT NULL							-- File size in bytes
	) ;


-- This table holds advertisemant
CREATE TABLE tbl_advertisement
	(
		id int NOT NULL PRIMARY KEY AUTO_INCREMENT, -- id for ad
		OwnerId int NOT NULL,						-- Ad owner id
		FilePathName varchar(128),					-- File path name of ad image
		AdUrl varchar(128)							-- Ad forwarding url
	) ;


-- This table holds model resource
CREATE TABLE tbl_model_desc
	(
		id int NOT NULL PRIMARY KEY AUTO_INCREMENT, -- id for model
		PCFileId int default -1,					-- File id for PC version
		MobileFileId int default -1,				-- File id for mobile version
		ModelName varchar(32),						-- Model name
		ModelDesc varchar(512),						-- Model description
		FOREIGN KEY (PCFileId) REFERENCES tbl_file_address(id),
		FOREIGN KEY (MobileFileId) REFERENCES tbl_file_address(id)
	) ;


-- This table holds owners
CREATE TABLE tbl_owner_desc
	(
		id int NOT NULL PRIMARY KEY AUTO_INCREMENT, -- id for owner
		FullName varchar(64),						-- Full name of owner
		Username varchar(64) NOT NULL,				-- Owner username
		Password varchar(64)						-- Owner password
	) ;


-- This table holds subscriptions
CREATE TABLE tbl_subscription
	(
		id int NOT NULL PRIMARY KEY AUTO_INCREMENT,			-- id for session entry
		OwnerId int,										-- Owner id for this subscription
		ModelId int,										-- Model id for this subscription
		AdId int,											-- Advertisement logo id
		HashId varchar(32) UNIQUE,
		Status int default -1,								-- Subscription status (active, expired, banned, etc.)
		FOREIGN KEY (OwnerId) REFERENCES tbl_owner_desc(id),
		FOREIGN KEY (ModelId) REFERENCES tbl_model_desc(id),
		FOREIGN KEY (AdId) REFERENCES tbl_advertisement(id)
	) ;

-- This table holds client types
CREATE TABLE tbl_client_type
	(
		ClientId varchar(64) PRIMARY KEY,			-- 
		ClientType int								--
	) ;


-- INSERT INTO tblDummySessionIds (SessionId, UserId) VALUES( 'Token aa12345654321bb', 1396 ) ;
-- INSERT INTO tblDummyProductIds (PackageName, ProductId) VALUES( 'alakigame', 34 ) ;
