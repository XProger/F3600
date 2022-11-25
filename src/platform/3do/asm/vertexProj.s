    AREA |C$$code|, CODE, READONLY
|x$codeseg|

    IMPORT gMatrixPtr
    IMPORT divTable

    EXPORT vertexProj_asm

src     RN r0
dst     RN r1
count   RN r2
last    RN count

tx      RN r3
ty      RN r4
tz      RN r5

x       RN r6
y       RN r7
z       RN r8
clip    RN r9

px      RN r10
py      RN r12
dz      RN py
gneg    RN py

xy      RN x
zp      RN y

m       RN py

divLUT  RN lr

FRAME_WIDTH     EQU 320
FRAME_HEIGHT    EQU 240
SUBPX_SHIFT     EQU 2
Z_NEAR          EQU (1 << 6)
Z_FAR           EQU (1 << 15)
VP_W            EQU ((FRAME_WIDTH >> 1) << SUBPX_SHIFT)
VP_H            EQU ((FRAME_HEIGHT >> 1) << SUBPX_SHIFT)
GUARDBAND       EQU (1024 << SUBPX_SHIFT)

vertexProj_asm
        stmfd sp!, {r4-r10, lr}

        add last, dst, count, lsl #3    ; last = dst + count * 8

        ldr m, =gMatrixPtr
        ldr m, [m]
        add m, m, #36           ; skip 3x3 matrix part
        ldmia m, {tx, ty, tz}   ; get view space offset from matrix
        ldr divLUT, =divTable

loop    ldmia src!, {x, y, z}   ; read view-space vertex coords

        add x, x, tx
        add y, y, ty
        add z, z, tz

        mov clip, #0

        cmp z, #Z_NEAR
        movlt z, #Z_NEAR
        movlt clip, #1
        cmp z, #Z_FAR
        movgt z, #Z_FAR
        movgt clip, #1

        mov z, z, lsr #3        ; z is positive
        ldr dz, [divLUT, z, lsl #2]
        mul px, x, dz
        mul py, y, dz

        mov x, px, asr #(12 - SUBPX_SHIFT)
        mov y, py, asr #(12 - SUBPX_SHIFT)

        add px, x, #VP_W
        cmp px, #(VP_W << 1)
        movhi clip, #1

        add py, y, #VP_H
        cmp py, #(VP_H << 1)
        movhi clip, #1

        mvn gneg, #GUARDBAND
        cmp x, gneg
        movlt x, gneg
        cmp y, gneg
        movlt y, gneg
        cmp x, #GUARDBAND
        movge x, #GUARDBAND
        cmp y, #GUARDBAND
        movge y, #GUARDBAND

        mov y, y, lsl #16
        mov y, y, lsr #16
        orr xy, y, x, lsl #16
        orr zp, clip, z, lsl #16

        stmia dst!, {xy, zp}    ; store projected vertex

        cmp dst, last
        blt loop

        ldmfd sp!, {r4-r10, pc}
    END
