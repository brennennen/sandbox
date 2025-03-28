# See section 18.6.2 of the buildroot manual
# Most packages should be pulled from separate version controlled projects with
# mechanisms like "{name}_SITE_METHOD = git". To keep this example simple, i'll just
# embed a simple c file and build steps here.
HELLO_SITE = '$(@D)/../../../hello/src'
HELLO_SITE_METHOD = local

define HELLO_BUILD_CMDS
	$(MAKE) $(TARGET_CONFIGURE_OPTS) -C $(@D) all
endef

define HELLO_INSTALL_TARGET_CMDS
	$(INSTALL) -m 0755 $(@D)/hello $(TARGET_DIR)/hello
endef

$(eval $(generic-package))
