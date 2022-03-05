#ifndef PSA_CRYPTO_CONFIG_H
#define PSA_CRYPTO_CONFIG_H

// <<< Use Configuration Wizard in Context Menu >>>

// <o SL_PSA_KEY_USER_SLOT_COUNT> PSA User Maximum Open Keys Count <0-128>
// <i> Maximum amount of keys that the user application will have open
// <i> simultaneously. In context of PSA Crypto, an open key means any key
// <i> either stored in RAM (lifetime set to PSA_KEY_LIFETIME_VOLATILE), or
// <i> used as part of a cryptographic operation.
// <i> When using a key for a multi-part (setup/update/finish) operation, a key
// <i> is considered to be open from the moment the operation is successfully
// <i> setup, until it finishes or aborts.
// <i> When an application tries to open more keys than this value accounts for,
// <i> the PSA API may return PSA_ERROR_INSUFFICIENT_MEMORY. Keep in mind that
// <i> other software included in the application (e.g. wireless protocol stacks)
// <i> also can have a need to have open keys in PSA Crypto. This could lead to
// <i> a race condition when the application key slot count is set too low for
// <i> the actual usage of the application, as a software stack may not fail
// <i> gracefully in case an application opens more than its declared amount of
// <i> keys, thereby precluding the stack from functioning.
// <i> Default: 4
#define SL_PSA_KEY_USER_SLOT_COUNT     (4)

// <o SL_PSA_ITS_USER_MAX_FILES> PSA Maximum User Persistent Keys Count <0-1024>
// <i> Maximum amount of keys (or other files) that can be stored persistently
// <i> by the application through the PSA interface, when persistent storage
// <i> support for PSA Crypto is included in the project.
// <i> Due to caching logic, this setting does have an impact on static RAM usage.
// <i> Note that this number is added to the potential requirements from other
// <i> software components in the project, such that the total amount of keys
// <i> which can be stored through the ITS backend can be higher than what is
// <i> configured here.
// <i>
// <i> WARNING: When changing this setting on an application that is already
// <i> deployed, and thus will get the change through an application upgrade,
// <i> care should be taken to ensure that the setting is only ever increased,
// <i> and never decreased. Decreasing this setting might cause previously
// <i> stored keys/files to become inaccessible.
// <i> Default: 128
#define SL_PSA_ITS_USER_MAX_FILES           (128)
// <<< end of configuration section >>>

// Include the autogenerated PSA Crypto configuration file
#include "psa_crypto_config_autogen.h"

#endif // PSA_CRYPTO_CONFIG_H