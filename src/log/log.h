#ifndef ERROR_LOG
	#define INFO_LOG
#endif

#ifdef TAG
	#define PRINT_TAG() Serial.print(":[" TAG "] ");
#else
	#define PRINT_TAG() Serial.print(": ");
#endif

#ifdef ENABLE_LOG
	#ifdef INFO_LOG
		#define LOG(...) \
			Serial.print(millis());\
			PRINT_TAG()\
			Serial.printf(__VA_ARGS__);\
			Serial.println();
	#endif

	#define LOGE(...) \
		Serial.print(millis());\
		PRINT_TAG()\
		Serial.printf(__VA_ARGS__);\
		Serial.println();
#endif

#ifndef LOG
	#define LOG(format, ...)
#endif
#ifndef LOGE
	#define LOGE(format, ...)
#endif
