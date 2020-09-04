add_mocked_test(adldap
	SOURCES test_adldap.c
	COMPILE_OPTIONS -g3 -I${CMAKE_CURRENT_SOURCE_DIR}/..
	LINK_LIBRARIES adldap)

