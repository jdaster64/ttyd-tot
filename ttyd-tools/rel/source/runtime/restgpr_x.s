.global _restgpr_14_x
.global _restgpr_15_x
.global _restgpr_16_x
.global _restgpr_17_x
.global _restgpr_18_x
.global _restgpr_19_x
.global _restgpr_20_x
.global _restgpr_21_x
.global _restgpr_22_x
.global _restgpr_23_x
.global _restgpr_24_x
.global _restgpr_25_x
.global _restgpr_26_x
.global _restgpr_27_x
.global _restgpr_28_x
.global _restgpr_29_x
.global _restgpr_30_x
.global _restgpr_31_x

_restgpr_14_x: lwz %r14,-0x48(%r11)
_restgpr_15_x: lwz %r15,-0x44(%r11)
_restgpr_16_x: lwz %r16,-0x40(%r11)
_restgpr_17_x: lwz %r17,-0x3C(%r11)
_restgpr_18_x: lwz %r18,-0x38(%r11)
_restgpr_19_x: lwz %r19,-0x34(%r11)
_restgpr_20_x: lwz %r20,-0x30(%r11)
_restgpr_21_x: lwz %r21,-0x2C(%r11)
_restgpr_22_x: lwz %r22,-0x28(%r11)
_restgpr_23_x: lwz %r23,-0x24(%r11)
_restgpr_24_x: lwz %r24,-0x20(%r11)
_restgpr_25_x: lwz %r25,-0x1C(%r11)
_restgpr_26_x: lwz %r26,-0x18(%r11)
_restgpr_27_x: lwz %r27,-0x14(%r11)
_restgpr_28_x: lwz %r28,-0x10(%r11)
_restgpr_29_x: lwz %r29,-0xC(%r11)
_restgpr_30_x: lwz %r30,-0x8(%r11)
_restgpr_31_x:
lwz %r0,0x4(%r11)
lwz %r31,-0x4(%r11)
mtlr %r0
mr %sp,%r11
blr