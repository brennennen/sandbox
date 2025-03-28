# See section 9.2.1 in the buildroot manual (take note of the "Example Layout" section for more complex scenarios)
include $(sort $(wildcard $(BR2_EXTERNAL_external_PATH)/packages/*/*.mk))
