#pragma once
/*
* Namespace defining constant values used in server and clients.
*/
namespace ClientServerConsts
{
	/* connection/disconnection codes */
	static const int CONNECT_CODE = -1; //code used to indicate making a connection
	static const int DISCONNECT_CODE = -2; //code used to indicate disconnecting
	static const int CONNECT_ERROR_CODE = -3; //code used to indicate an error in connecting
	static const int DISCONNECT_SUCCESS_CODE = -4; //code used to indicate a successful disconnection
	static const int CONNECTION_FILLER = -5; //filler to fill up unused params

	/* server socket ports */
	static const char* CONNECT_DISCONNECT_REQ_REP_SERVER_PORT = "tcp://*:5555"; //port for receiving connection/disconnection requests
	static const char* CONNECT_DISCONNECT_PUB_SUB_SERVER_PORT = "tcp://*:5556"; //port for publishing connections/disconnections
	static const char* MOVEMENT_UPDATE_PUB_SUB_SERVER_PORT = "tcp://*:5557"; //port for publishing changes in positions of objects
	static const char* OBJECT_CREATION_REQ_REP_SERVER_PORT = "tcp://*:5560"; //port for sending object creation messages to connecting clients
	static const char* EVENT_RAISING_REQ_REP_SERVER_PORT_START = "tcp://*:"; //port start for receiving event raising messages from clients
	static const int EVENT_RAISING_REQ_REP_NUM_START = 5561; //starting number for event raising ports

	/* client socket ports */
	static const char* CONNECT_DISCONNECT_REQ_REP_CLIENT_PORT = "tcp://localhost:5555"; //port for sending connection/disconnection requests
	static const char* CONNECT_DISCONNECT_PUB_SUB_CLIENT_PORT = "tcp://localhost:5556"; //port for receiving published connections/disconnections
	static const char* MOVEMENT_UPDATE_PUB_SUB_CLIENT_PORT = "tcp://localhost:5557"; //port for receiving published positional updates
	static const char* OBJECT_CREATION_REQ_REP_CLIENT_PORT = "tcp://localhost:5560"; //port for receiving object creation messages from server while connecting
	static const char* EVENT_RAISING_REQ_REP_CLIENT_PORT_START = "tcp://localhost:"; //port start for sending event raising messages to server

	/* window attributes */
	static const int WINDOW_WIDTH = 800; //width of window
	static const int WINDOW_HEIGHT = 600; //height of window
	static const char* WINDOW_TITLE = "CSC 481 - HW4";

	/*platform attributes*/
	static const float PLATFORM_WIDTH = 100.f;
	static const float PLATFORM_HEIGHT = 50.f;
	static const float PLATFORM_PAUSE_DURATION = 2000.f;
	static const float PLATFORM_VELOCITY = 0.2f;

	/*character attributes*/
	static const float CHARACTER_RADIUS = 50.f;
	static const int CHARACTER_NUM_OF_POINTS = 100;
	static const float CHARACTER_VELOCITY = 3.f;

	/*property and object types*/
	static const int PROPERTY_LOCATION_IN_SPACE = 0;
	static const int PROPERTY_RENDERING = 1;
	static const int STATIC_PLATFORM = 2;
	static const int MOVING_PLATFORM = 3;
	static const int CHARACTER = 4;

	/*filler code for object creation*/
	static const int CREATION_FILLER = 0;

	/*number of params in creation message*/
	static const int CREATION_PARAM_NUM = 7;

	/*code indicating end of object/property creation messages*/
	static const int CREATION_END = -1;

	/* event types */
	static const std::string CLIENT_DISCONNECT_EVENT = "ClientDisconnectEvent"; //no args; sent by client to stop thread for its event raising
	static const std::string USER_INPUT_EVENT = "UserInputEvent"; //args: character id and key type
	static const std::string CHARACTER_COLLISION_EVENT = "CharacterCollisionEvent"; //args: character id, colliding object id
	static const std::string CHARACTER_DEATH_EVENT = "CharacterDeathEvent"; //args: character id
	static const std::string CHARACTER_SPAWN_EVENT = "CharacterSpawnEvent"; //args: character id, location in space id, new absolute x, new absolute y
	static const std::string CHARACTER_MOVED_EVENT = "CharacterMovedEvent"; /*args: character id, location in space id,
																			x moved, y moved, new absolute x, new absolute y*/
	static const std::string PLATFORM_MOVED_EVENT = "PlatformMovedEvent"; /*args: platform id, location in space id, 
																			x moved, y moved, new absolute x, new absolute y*/
	static const std::string CHARACTER_JUMP_START_EVENT = "CharacterJumpStartEvent"; //args: character id
	static const std::string CHARACTER_STILL_JUMPING_EVENT = "CharacterStillJumpingEvent"; //args: character id
	static const std::string CHARACTER_JUMP_END_EVENT = "CharacterJumpEndEvent"; //args: character id
	static const std::string CHARACTER_FALL_START_EVENT = "CharacterFallStartEvent"; //args: character id
	static const std::string CHARACTER_STILL_FALLING_EVENT = "CharacterStillFallingEvent"; //args: character id
	static const std::string CHARACTER_FALL_END_EVENT = "CharacterFallEndEvent"; //args: character id
	static const std::string CHARACTER_MOVED_BY_GRAVITY_EVENT = "CharacterMovedByGravityEvent"; /*args: character id, location in space id,
																								x moved, y moved, new absolute x, new absolute y*/
	static const std::string CHARACTER_MOVED_BY_PLATFORM_EVENT = "CharacterMovedByPlatformEvent"; /*args: character id, location in space id,
																								x moved, y moved, new absolute x, new absolute y*/
	static const std::string REPLAY_RECORDING_START_EVENT = "ReplayRecordingStartEvent"; //args: none
	static const std::string REPLAY_RECORDING_STOP_EVENT = "ReplayRecordingStopEvent"; //args: initial replay speed

	/* event type codes (for client sending to server) */
	static const int CLIENT_DISCONNECT_EVENT_CODE = 1;
	static const int USER_INPUT_EVENT_CODE = 2;

	/* key types for user input */
	static const int LEFT_ARROW_KEY = 1;
	static const int RIGHT_ARROW_KEY = 2;
	static const int UP_ARROW_KEY = 3;
	static const int R_KEY = 4;
	static const int ONE_KEY = 5;
	static const int TWO_KEY = 6;
	static const int THREE_KEY = 7;
}