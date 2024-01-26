.global _restfpr_14_x
.global _restfpr_15_x
.global _restfpr_16_x
.global _restfpr_17_x
.global _restfpr_18_x
.global _restfpr_19_x
.global _restfpr_20_x
.global _restfpr_21_x
.global _restfpr_22_x
.global _restfpr_23_x
.global _restfpr_24_x
.global _restfpr_25_x
.global _restfpr_26_x
.global _restfpr_27_x
.global _restfpr_28_x
.global _restfpr_29_x
.global _restfpr_30_x
.global _restfpr_31_x

_restfpr_14_x: lfd %f14,-0x90(%r11)
_restfpr_15_x: lfd %f15,-0x88(%r11)
_restfpr_16_x: lfd %f16,-0x80(%r11)
_restfpr_17_x: lfd %f17,-0x78(%r11)
_restfpr_18_x: lfd %f18,-0x70(%r11)
_restfpr_19_x: lfd %f19,-0x68(%r11)
_restfpr_20_x: lfd %f20,-0x60(%r11)
_restfpr_21_x: lfd %f21,-0x58(%r11)
_restfpr_22_x: lfd %f22,-0x50(%r11)
_restfpr_23_x: lfd %f23,-0x48(%r11)
_restfpr_24_x: lfd %f24,-0x40(%r11)
_restfpr_25_x: lfd %f25,-0x38(%r11)
_restfpr_26_x: lfd %f26,-0x30(%r11)
_restfpr_27_x: lfd %f27,-0x28(%r11)
_restfpr_28_x: lfd %f28,-0x20(%r11)
_restfpr_29_x: lfd %f29,-0x18(%r11)
_restfpr_30_x: lfd %f30,-0x10(%r11)
_restfpr_31_x:
lwz %r0,0x4(%r11)
lfd %f31,-0x8(%r11)
mtlr %r0
mr %sp,%r11
blr