-- ------------------------------------------------------------------------------------
--                                                                                    -
--   Copyright(C) 2017 Ali Seyedof	(seyedof@gmail.com)								  -
--                                                                                    -
-- ------------------------------------------------------------------------------------

-- --------------------------------------------
-- Create tables
-- --------------------------------------------

-- Change the database name to match your own pre-created database
USE dbModelPublish ;


-- This table holds product (game) purchase keys
CREATE TABLE tblPurchaseKeys
	(
		id int NOT NULL PRIMARY KEY AUTO_INCREMENT, -- id for each key
		PurchaseId int NOT NULL UNIQUE,				-- Purchase id (to be used for integration with existing store back-end)
		UserId int NOT NULL,						-- User id (int) who owns this purchase key
		ProductId int NOT NULL,						-- Product (game) id (int) which has been purchased
		PublicKey varchar(128),						-- Public key string assigned to this purchase
		PrivateKey varchar(128),					-- Private key string assigned to this purchase
		RegistrationKey	nvarchar(256),				-- Registration key to be represented/published/displayed in a human readable form
		ScrambleKey	varchar(1024),					-- Scramble key used to generate CEG for this purchase
		Flags int DEFAULT 0							-- Reserved bitfield for flags, to be used later (e.g. ban a purchage, purchase type,...)
	) ;


-- This is a dummy table to map session id strings to user id, for dev time only, remove in final deployment.
CREATE TABLE tblDummySessionIds
	(
		id int NOT NULL PRIMARY KEY AUTO_INCREMENT, -- id for session entry
		SessionId varchar(128),						-- Session id string
		UserId int NOT NULL							-- User id (int) who owns this session
	) ;

-- This is a dummy table to map product token strings to product id, for dev time only, remove in final deployment.
CREATE TABLE tblDummyProductIds
	(
		id int NOT NULL PRIMARY KEY AUTO_INCREMENT, -- id for product entry
		PackageName varchar(128),					-- Package name string
		ProductId int NOT NULL						-- Product id (int)
	) ;

-- This is a dummy table to map purchase id to order id string, for dev time only, remove in final deployment.
CREATE TABLE tblDummyOrderIds
	(
		id int NOT NULL PRIMARY KEY AUTO_INCREMENT, -- id for purchase entry
		PurchaseId int NOT NULL,					-- Purchase id (int)
		OrderId varchar(32)						-- Order id string(32)
	) ;


-- INSERT INTO tblDummySessionIds (SessionId, UserId) VALUES( 'Token aa12345654321bb', 1396 ) ;
-- INSERT INTO tblDummyProductIds (PackageName, ProductId) VALUES( 'alakigame', 34 ) ;
