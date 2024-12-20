set(COMPONENT_ADD_INCLUDEDIRS "include")
set(COMPONENT_SRCS "Attribute.cpp" "Endpoint.cpp" "Discovery.cpp" "utils.cpp")
set(COMPONENT_REQUIRES "json" "MqttCpp")

register_component()
