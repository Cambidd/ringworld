.intel_syntax noprefix

.text

.globl _hud_calculate_font_height_asm
_hud_calculate_font_height_asm:
    pushad
    pushfd
    mov cl, byte ptr [esp + 0x50]
    push ecx
    push eax
    call _hud_calculate_font_height
    add esp, 0x8
    mov dword ptr [esp + 0x1C], eax
    popfd
    popad
    ret

.global _damage_indicator_update_screen_width_asm
_damage_indicator_update_screen_width_asm:
    pushad
    pushfd
    call _damage_indicators_screen_dimensions_hack
    popfd
    popad
    mov ecx, dword ptr [_damage_indicator_screen_width]
    jmp dword ptr [_damage_indicator_hack_ret]