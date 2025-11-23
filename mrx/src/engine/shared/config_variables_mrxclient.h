// MRX_CLIENT
 // AvoidFreeze (Legit‑style planner with sub-tick simulation)
 MACRO_CONFIG_INT(MRXAvoidEnable, mrx_avoid_enable, 0, 0, 1, CFGFLAG_CLIENT | CFGFLAG_SAVE, "Enable MRX AvoidFreeze")
 MACRO_CONFIG_INT(MRXAvoidHookAssist, mrx_avoid_hook_assist, 1, 0, 1, CFGFLAG_CLIENT | CFGFLAG_SAVE, "Allow bot to press/release hook")
 MACRO_CONFIG_INT(MRXAvoidDirectionAssist, mrx_avoid_direction_assist, 1, 0, 1, CFGFLAG_CLIENT | CFGFLAG_SAVE, "Allow bot to change move direction")
 MACRO_CONFIG_INT(MRXAvoidCheckTicks, mrx_avoid_check_ticks, 8, 1, 20, CFGFLAG_CLIENT | CFGFLAG_SAVE, "How many ticks to simulate ahead")
 MACRO_CONFIG_INT(MRXAvoidCheckTicksHook, mrx_avoid_check_ticks_hook, 8, 1, 30, CFGFLAG_CLIENT | CFGFLAG_SAVE, "How many ticks to simulate ahead for hook")

 //MACRO_CONFIG_INT(MRXAvoidSubsteps, mrx_avoid_substeps, 4, 1, 100, CFGFLAG_CLIENT | CFGFLAG_SAVE, "Sub-steps inside each physics tick")
 //MACRO_CONFIG_INT(MRXAvoidQuality, mrx_avoid_quality, 2, 1, 1000, CFGFLAG_CLIENT | CFGFLAG_SAVE, "Quality level (affects candidates/substeps sweep)")
 //MACRO_CONFIG_INT(MRXAvoidRandomness, mrx_avoid_randomness, 10, 0, 100, CFGFLAG_CLIENT | CFGFLAG_SAVE, "Randomness percent to break ties")
 //MACRO_CONFIG_INT(MRXAvoidDirectionPriority, mrx_avoid_direction_priority, 200, 0, 1000, CFGFLAG_CLIENT | CFGFLAG_SAVE, "Weight for keeping player's desired direction")
 //MACRO_CONFIG_INT(MRXAvoidHookPriority, mrx_avoid_hook_priority, 500, 0, 1000, CFGFLAG_CLIENT | CFGFLAG_SAVE, "Weight for keeping player's hook state")
 //MACRO_CONFIG_INT(MRXAvoidLifePriority, mrx_avoid_life_priority, 1000, 1, 1000, CFGFLAG_CLIENT | CFGFLAG_SAVE, "Weight for survival (avoid freeze)")
 
 // Per-tile toggles (preferred over legacy mask)
 MACRO_CONFIG_INT(MRXAvoidTileFreeze, mrx_avoid_tile_freeze, 1, 0, 1, CFGFLAG_CLIENT | CFGFLAG_SAVE, "Avoid Freeze tiles")
 MACRO_CONFIG_INT(MRXAvoidTileDeath, mrx_avoid_tile_death, 0, 0, 1, CFGFLAG_CLIENT | CFGFLAG_SAVE, "Avoid Death tiles")
 MACRO_CONFIG_INT(MRXAvoidTileTele, mrx_avoid_tile_tele, 0, 0, 1, CFGFLAG_CLIENT | CFGFLAG_SAVE, "Avoid Teleporter tiles")
 MACRO_CONFIG_INT(MRXAvoidTileUnfreeze, mrx_avoid_tile_unfreeze, 0, 0, 1, CFGFLAG_CLIENT | CFGFLAG_SAVE, "Avoid Unfreeze tiles")

 
 // Legacy bit mask kept for backward compatibility; not used by UI anymore
 MACRO_CONFIG_INT(MRXAvoidTilesMask, mrx_avoid_tiles_mask, 3, 0, 15, CFGFLAG_CLIENT | CFGFLAG_SAVE, "[deprecated] Danger tiles mask: 1=Freeze,2=Death,4=Tele,8=Unfreeze")
 
 //AUTOPSEUDO
 MACRO_CONFIG_INT(MRXDummyPseudo, mrx_autopseudo, 0, 0, 1, CFGFLAG_CLIENT | CFGFLAG_SAVE, "Enable auto pseudo")
 MACRO_CONFIG_INT(MRXCopy_nomove, mrx_copy_nomove, 0, 0, 1, CFGFLAG_CLIENT | CFGFLAG_SAVE, "Copy pseudo on join only if you don't move")
 MACRO_CONFIG_INT(MRXCopy, mrx_copy, 0, 0, 1, CFGFLAG_CLIENT | CFGFLAG_SAVE, "Copy pseudo on join only if you don't change it")
MACRO_CONFIG_INT(MRXTargetHit, mrx_autopseudo_target_hit, 0, 0, 200, CFGFLAG_CLIENT | CFGFLAG_SAVE, "Hold dummy aim on player for N ticks after hammer hit (0=disabled)")
 
 // ArrayList settings
 MACRO_CONFIG_INT(MRXArrayList, mrx_arraylist, 1, 0, 1, CFGFLAG_CLIENT | CFGFLAG_SAVE, "ArrayList")
 MACRO_CONFIG_INT(MRXArrayListGradient, mrx_arraylist_gradient, 1, 0, 1, CFGFLAG_CLIENT | CFGFLAG_SAVE, "ArrayList Gradient")
 MACRO_CONFIG_INT(MRXArrayListLine, mrx_arraylist_line, 1, 0, 1, CFGFLAG_CLIENT | CFGFLAG_SAVE, "ArrayList Line")
 MACRO_CONFIG_INT(MRXArrayListMask, mrx_arraylist_mask, 0x3FFFF, 0, 0x3FFFF, CFGFLAG_CLIENT | CFGFLAG_SAVE, "ArrayList visible features mask")
 MACRO_CONFIG_INT(MRXArrayListX, mrx_arraylist_x, 5000, 0, 20000, CFGFLAG_CLIENT | CFGFLAG_SAVE, "ArrayList position X percent")
 MACRO_CONFIG_INT(MRXArrayListY, mrx_arraylist_y, 5000, 0, 20000, CFGFLAG_CLIENT | CFGFLAG_SAVE, "ArrayList position Y percent")
 
 // MRX Overlay Text
 MACRO_CONFIG_INT(MRXOverlayText, mrx_overlaytext, 0, 0, 1, CFGFLAG_CLIENT | CFGFLAG_SAVE, "Enable MRX overlay text")
 MACRO_CONFIG_INT(MRXOverlayTextSize, mrx_overlaytext_size, 14, 6, 72, CFGFLAG_CLIENT | CFGFLAG_SAVE, "Font size for MRX overlay text")
 MACRO_CONFIG_COL(MRXOverlayTextColor, mrx_overlaytext_color, 0xFFFFFFFF, CFGFLAG_CLIENT | CFGFLAG_SAVE, "Text color for MRX overlay text")
 
 // 5 overlay text slots: base + 1..4
 MACRO_CONFIG_STR(MRXText, mrx_text, 256, "", CFGFLAG_CLIENT | CFGFLAG_SAVE, "MRX overlay text 0")
 MACRO_CONFIG_INT(MRXTextX, mrx_text_x, 5000, 0, 10000, CFGFLAG_CLIENT | CFGFLAG_SAVE, "MRX overlay text 0 pos X percent")
 MACRO_CONFIG_INT(MRXTextY, mrx_text_y, 5000, 0, 10000, CFGFLAG_CLIENT | CFGFLAG_SAVE, "MRX overlay text 0 pos Y percent")
 MACRO_CONFIG_COL(MRXTextColor, mrx_text_color, 0xFFFFFFFF, CFGFLAG_CLIENT | CFGFLAG_SAVE, "Text 0 color")
 
 MACRO_CONFIG_STR(MRXText1, mrx_text1, 256, "", CFGFLAG_CLIENT | CFGFLAG_SAVE, "MRX overlay text 1")
 MACRO_CONFIG_INT(MRXText1X, mrx_text1_x, 5000, 0, 10000, CFGFLAG_CLIENT | CFGFLAG_SAVE, "MRX overlay text 1 pos X percent")
 MACRO_CONFIG_INT(MRXText1Y, mrx_text1_y, 5000, 0, 10000, CFGFLAG_CLIENT | CFGFLAG_SAVE, "MRX overlay text 1 pos Y percent")
 MACRO_CONFIG_COL(MRXText1Color, mrx_text1_color, 0xFFFFFFFF, CFGFLAG_CLIENT | CFGFLAG_SAVE, "Text 1 color")
 
 MACRO_CONFIG_STR(MRXText2, mrx_text2, 256, "", CFGFLAG_CLIENT | CFGFLAG_SAVE, "MRX overlay text 2")
 MACRO_CONFIG_INT(MRXText2X, mrx_text2_x, 5000, 0, 10000, CFGFLAG_CLIENT | CFGFLAG_SAVE, "MRX overlay text 2 pos X percent")
 MACRO_CONFIG_INT(MRXText2Y, mrx_text2_y, 5000, 0, 10000, CFGFLAG_CLIENT | CFGFLAG_SAVE, "MRX overlay text 2 pos Y percent")
 MACRO_CONFIG_COL(MRXText2Color, mrx_text2_color, 0xFFFFFFFF, CFGFLAG_CLIENT | CFGFLAG_SAVE, "Text 2 color")
 
 MACRO_CONFIG_STR(MRXText3, mrx_text3, 256, "", CFGFLAG_CLIENT | CFGFLAG_SAVE, "MRX overlay text 3")
 MACRO_CONFIG_INT(MRXText3X, mrx_text3_x, 5000, 0, 10000, CFGFLAG_CLIENT | CFGFLAG_SAVE, "MRX overlay text 3 pos X percent")
 MACRO_CONFIG_INT(MRXText3Y, mrx_text3_y, 5000, 0, 10000, CFGFLAG_CLIENT | CFGFLAG_SAVE, "MRX overlay text 3 pos Y percent")
 MACRO_CONFIG_COL(MRXText3Color, mrx_text3_color, 0xFFFFFFFF, CFGFLAG_CLIENT | CFGFLAG_SAVE, "Text 3 color")
 
 MACRO_CONFIG_STR(MRXText4, mrx_text4, 256, "", CFGFLAG_CLIENT | CFGFLAG_SAVE, "MRX overlay text 4")
 MACRO_CONFIG_INT(MRXText4X, mrx_text4_x, 5000, 0, 10000, CFGFLAG_CLIENT | CFGFLAG_SAVE, "MRX overlay text 4 pos X percent")
 MACRO_CONFIG_INT(MRXText4Y, mrx_text4_y, 5000, 0, 10000, CFGFLAG_CLIENT | CFGFLAG_SAVE, "MRX overlay text 4 pos Y percent")
 MACRO_CONFIG_COL(MRXText4Color, mrx_text4_color, 0xFFFFFFFF, CFGFLAG_CLIENT | CFGFLAG_SAVE, "Text 4 color")

 // Extra hint: show message when local player shares X with someone (above/below)
 MACRO_CONFIG_INT(MRXOverlaySameX, mrx_overlay_samex, 0, 0, 1, CFGFLAG_CLIENT | CFGFLAG_SAVE, "Enable Same-X hint message")
 MACRO_CONFIG_INT(MRXOverlaySameXX, mrx_overlay_samex_x, 5000, 0, 10000, CFGFLAG_CLIENT | CFGFLAG_SAVE, "Same-X hint X position percent")
 MACRO_CONFIG_INT(MRXOverlaySameXY, mrx_overlay_samex_y, 800, 0, 10000, CFGFLAG_CLIENT | CFGFLAG_SAVE, "Same-X hint Y position percent")
 
 // Movefly settings
 MACRO_CONFIG_INT(MRXMoveflyBotDistance, mrx_movefly_distance, 10, 0, 50, CFGFLAG_CLIENT | CFGFLAG_SAVE, "Movefly Distance")
 MACRO_CONFIG_INT(MRXMoveflyBot, mrx_movefly, 0, 0, 1, CFGFLAG_CLIENT | CFGFLAG_SAVE, "Movefly")
 MACRO_CONFIG_INT(MRXMoveflyBotDirect, mrx_moveflydir, 0, 0, 1, CFGFLAG_CLIENT | CFGFLAG_SAVE, "Moveflydir")

 // Misc settingsm_MRXCursorScale
 MACRO_CONFIG_INT(MRXPixelWalk, mrx_pixelwalk, 0, 0, 1, CFGFLAG_CLIENT | CFGFLAG_SAVE, "PixelWalk")
 MACRO_CONFIG_INT(MRXBalanceBot, mrx_balancebot, 0, 0, 1, CFGFLAG_CLIENT | CFGFLAG_SAVE, "BalanceBot")
 MACRO_CONFIG_INT(MRXBalanceBotVFilter, mrx_balancebot_filter, 0, 0, 2, CFGFLAG_CLIENT | CFGFLAG_SAVE, "BalanceBot Filter")
 MACRO_CONFIG_INT(MRXBalanceBotPrecise, mrx_balancebot_precise, 0, 0, 2, CFGFLAG_CLIENT | CFGFLAG_SAVE, "BalanceBot Precise (0=Normal,1=Precise,2=Legit)")
 MACRO_CONFIG_INT(MRXBalanceBotLegit, mrx_balancebot_legit, 0, 0, 1, CFGFLAG_CLIENT | CFGFLAG_SAVE, "BalanceBot Legit")
 MACRO_CONFIG_INT(MRXDummyHookCursor, mrx_dummy_hookcursor, 0, 0, 1, CFGFLAG_CLIENT| CFGFLAG_SAVE, "DummyHookCursor")
 MACRO_CONFIG_INT(MRXAdvancedDeepFly, mrx_advancedeepfly, 0, 0, 1, CFGFLAG_CLIENT | CFGFLAG_SAVE, "Advanced deepfly via fire button")
 MACRO_CONFIG_INT(MRXAdvancedDeepFlyManual, mrx_advanceddeepfly_manual, 0, 0, 1, CFGFLAG_CLIENT | CFGFLAG_SAVE, "Advanced DeepFly")
 MACRO_CONFIG_INT(MRXDummyAled, mrx_dummyaled, 0, 0, 1, CFGFLAG_CLIENT | CFGFLAG_SAVE, "dummy aled")
 MACRO_CONFIG_INT(MRXAutoAled, mrx_autoaled, 0, 0, 1, CFGFLAG_CLIENT | CFGFLAG_SAVE, "aled")
 MACRO_CONFIG_INT(MRXAutoAledSilent, mrx_autoaled_silent, 0, 0, 1, CFGFLAG_CLIENT | CFGFLAG_SAVE, "auto aled silent")
 MACRO_CONFIG_INT(MRXAutoHookDummy, mrx_autohookdummy, 0, 0, 1, CFGFLAG_CLIENT | CFGFLAG_SAVE, "auto hook dummy")
 MACRO_CONFIG_INT(MRXShowRealAngle, mrx_show_real_angle, 0, 0, 1, CFGFLAG_CLIENT | CFGFLAG_SAVE, "Show real angle")
 MACRO_CONFIG_INT(MRXHookFlyAuto, mrx_hookfly_auto, 0, 0, 1, CFGFLAG_CLIENT|CFGFLAG_SAVE, "Enable automatic hookfly (dummy hooks player when player is below)")
 MACRO_CONFIG_INT(MRXHookFlyManual, mrx_hookfly_manual, 0, 0, 1, CFGFLAG_CLIENT|CFGFLAG_SAVE, "Enable manual hookfly (dummy hooks player on command)")
 MACRO_CONFIG_INT(MRXHookFlyAutoDistance, mrx_hookfly_autodistance, 10, 0, 250, CFGFLAG_CLIENT|CFGFLAG_SAVE, "Automatic hookfly distance")
 MACRO_CONFIG_INT(MRXNullMovement, mrx_null_movement, 0, 0, 1, CFGFLAG_CLIENT | CFGFLAG_SAVE, "Null Movement")
 MACRO_CONFIG_INT(MRXChatAnim, mrx_anim_chat, 0, 0, 1, CFGFLAG_CLIENT | CFGFLAG_SAVE, "anim chat")
 MACRO_CONFIG_INT(MRXCursorScale, mrx_cursor_scale, 100, 5, 500, CFGFLAG_CLIENT | CFGFLAG_SAVE, "Cursor Scale")
MACRO_CONFIG_INT(MRXThrowDummy, mrx_throw_dummy, 0, 0, 1, CFGFLAG_CLIENT | CFGFLAG_SAVE, "Throw Dummy")
MACRO_CONFIG_INT(MRXFreezeSwitchFire, mrx_freeze_switch_fire, 0, 0, 1, CFGFLAG_CLIENT | CFGFLAG_SAVE, "Dummy Freeze Fire")
MACRO_CONFIG_INT(MRXSmartDummyHook, mrx_smart_dummy_hook, 0, 0, 1, CFGFLAG_CLIENT | CFGFLAG_SAVE, "Smart Dummy Hook - only hook when moving upward (VelY < 0), works with DummyHookCursor")
MACRO_CONFIG_INT(MRXFireCheckRadius, mrx_fire_check_radius, 0, 0, 1, CFGFLAG_CLIENT | CFGFLAG_SAVE, "Fire only when player is in radius 63 units")
 
// Trails / playersvisual
 MACRO_CONFIG_INT(MRXTeeTrail, mrx_tee_trail, 0, 0, 1, CFGFLAG_CLIENT | CFGFLAG_SAVE, "Enable Tee trails")
 MACRO_CONFIG_INT(MRXTeeTrailOthers, mrx_tee_trail_others, 1, 0, 1, CFGFLAG_CLIENT | CFGFLAG_SAVE, "Show tee trails for other players")
 MACRO_CONFIG_INT(MRXTeeTrailWidth, mrx_tee_trail_width, 15, 0, 20, CFGFLAG_CLIENT | CFGFLAG_SAVE, "Tee trail width")
 MACRO_CONFIG_INT(MRXTeeTrailLength, mrx_tee_trail_length, 25, 5, 200, CFGFLAG_CLIENT | CFGFLAG_SAVE, "Tee trail length")
 MACRO_CONFIG_INT(MRXTeeTrailAlpha, mrx_tee_trail_alpha, 80, 1, 100, CFGFLAG_CLIENT | CFGFLAG_SAVE, "Tee trail alpha")
 MACRO_CONFIG_COL(MRXTeeTrailColor, mrx_tee_trail_color, 255, CFGFLAG_CLIENT | CFGFLAG_SAVE, "Tee trail color")
 MACRO_CONFIG_INT(MRXTeeTrailTaper, mrx_tee_trail_taper, 1, 0, 1, CFGFLAG_CLIENT | CFGFLAG_SAVE, "Taper tee trail over length")
 MACRO_CONFIG_INT(MRXTeeTrailFade, mrx_tee_trail_fade, 0, 0, 1, CFGFLAG_CLIENT | CFGFLAG_SAVE, "Fade trail alpha over length")
 MACRO_CONFIG_INT(MRXTeeTrailColorMode, mrx_tee_trail_color_mode, 1, 1, 5, CFGFLAG_CLIENT | CFGFLAG_SAVE, "Tee trail color mode (1: Solid color, 2: Current Tee color, 3: Rainbow, 4: Color based on Tee speed, 5: Random)")
 MACRO_CONFIG_INT(MRXRemoveAnti, mrx_remove_anti, 0, 0, 1, CFGFLAG_CLIENT | CFGFLAG_SAVE, "Removes some amount of antiping & player prediction in freeze")
 MACRO_CONFIG_INT(MRXUnpredOthersInFreeze, mrx_unpred_others_in_freeze, 0, 0, 1, CFGFLAG_CLIENT | CFGFLAG_SAVE, "Dont predict other players if you are frozen")
 MACRO_CONFIG_INT(MRXUnfreezeLagTicks, mrx_unfreezelagtick, 0, 0, 1, CFGFLAG_CLIENT | CFGFLAG_SAVE, "Dont predict other players if you are frozen")
 MACRO_CONFIG_INT(MRXFastInput, mrx_fastinp, 0, 0, 1, CFGFLAG_CLIENT | CFGFLAG_SAVE, "Dont predict other players if you are frozen")
 MACRO_CONFIG_INT(MRXFastInputOthers, mrx_fastinpother, 0, 0, 1, CFGFLAG_CLIENT | CFGFLAG_SAVE, "Dont predict other players if you are frozen")
 MACRO_CONFIG_INT(MRXUnfreezeLagDelayTicks, mrx_unflagdeltick, 0, 0, 1, CFGFLAG_CLIENT | CFGFLAG_SAVE, "Dont predict other players if you are frozen")

 //katan
 MACRO_CONFIG_INT(MRXFreezeKatana, mrx_frozen_katana, 0, 0, 1, CFGFLAG_CLIENT | CFGFLAG_SAVE, "Show katana on frozen players (reverts a DDNet change)")
 MACRO_CONFIG_INT(MRXColorFreeze, mrx_color_freeze, 0, 0, 1, CFGFLAG_CLIENT | CFGFLAG_SAVE, "Use skin colors for frozen tees")
 MACRO_CONFIG_INT(MRXColorFreezeDarken, mrx_color_freeze_darken, 90, 0, 100, CFGFLAG_CLIENT | CFGFLAG_SAVE, "Makes color of tees darker when in freeze (0-100)")
 MACRO_CONFIG_INT(MRXColorFreezeFeet, mrx_color_freeze_feet, 0, 0, 1, CFGFLAG_CLIENT | CFGFLAG_SAVE, "Also use color for frozen tee feet")

 // MRX: Center cross lines
 MACRO_CONFIG_INT(MRXCenterEnable, mrx_center_enable, 0, 0, 1, CFGFLAG_CLIENT | CFGFLAG_SAVE, "Enable MRX center cross lines")
 MACRO_CONFIG_INT(MRXCenterWidth, mrx_center_width, 0, 0, 40, CFGFLAG_CLIENT | CFGFLAG_SAVE, "Center lines thickness in pixels (0 = hairline)")
 MACRO_CONFIG_COL(MRXCenterColor, mrx_center_color, 0xFFFFFFFF, CFGFLAG_CLIENT | CFGFLAG_SAVE, "Center lines color")
 
 // Predicted trajectory lines
 MACRO_CONFIG_INT(MRXPredictTrajTicks, mrx_predict_traj_ticks, 20, 0, 200, CFGFLAG_CLIENT | CFGFLAG_SAVE, "Number of predicted ticks for trajectory line")
 MACRO_CONFIG_INT(MRXPredictTraj, mrx_predict_traj, 0, 0, 1, CFGFLAG_CLIENT | CFGFLAG_SAVE, "Enable predicted trajectory line rendering")
 MACRO_CONFIG_COL(MRXPredictTrajColor, mrx_predict_traj_color, 0x2E8FD8FF, CFGFLAG_CLIENT | CFGFLAG_SAVE, "Color of predicted trajectory line")
 MACRO_CONFIG_COL(MRXPredictTrajColorFreeze, mrx_predict_traj_color_freeze, 0xFF2A2AFF, CFGFLAG_CLIENT | CFGFLAG_SAVE, "Color of trajectory line when touching freeze")
 // 0 = solid, 1 = dashed
 MACRO_CONFIG_INT(MRXPredictTrajMode, mrx_predict_traj_mode, 0, 0, 1, CFGFLAG_CLIENT | CFGFLAG_SAVE, "Trajectory line mode (0=solid, 1=dashed)")
 
 MACRO_CONFIG_INT(MRXSlowWalk, mrx_slow_walk, 0, 0, 1, CFGFLAG_CLIENT | CFGFLAG_SAVE, "slow walk")
 
 //mrx_menus show section
 MACRO_CONFIG_INT(MRXShowBalanceBot, mrx_show_balancebot, 0, 0, 1, CFGFLAG_CLIENT | CFGFLAG_SAVE, "")
 MACRO_CONFIG_INT(MRXShowMoveFlyBot, mrx_show_moveflybot, 0, 0, 1, CFGFLAG_CLIENT | CFGFLAG_SAVE, "")
 MACRO_CONFIG_INT(MRXShowPredictTraj, mrx_show_predicttraj, 0, 0, 1, CFGFLAG_CLIENT | CFGFLAG_SAVE, "Show Predict Trajectory settings section")
 MACRO_CONFIG_INT(MRXShowAutoTripleFly, mrx_show_autotriplefly, 0, 0, 1, CFGFLAG_CLIENT | CFGFLAG_SAVE, "")
 MACRO_CONFIG_INT(MRXShowArrayList, mrx_show_arraylist, 0, 0, 1, CFGFLAG_CLIENT | CFGFLAG_SAVE, "")
 MACRO_CONFIG_INT(MRXShowBackgroundLines, mrx_show_backgourndlines, 0, 0, 1, CFGFLAG_CLIENT | CFGFLAG_SAVE, "")
 MACRO_CONFIG_INT(MRXShowHookFly, mrx_show_hookfly, 0, 0, 1, CFGFLAG_CLIENT | CFGFLAG_SAVE, "")
 
 //*Dummy copy
 MACRO_CONFIG_INT(MRXDummyCopyDirection, mrx_dummy_copy_direction, 0, 0, 1, CFGFLAG_CLIENT | CFGFLAG_SAVE, "Copy direction from main player to dummy")
 MACRO_CONFIG_INT(MRXDummyCopyHook, mrx_dummy_copy_hook, 0, 0, 1, CFGFLAG_CLIENT | CFGFLAG_SAVE, "Copy hook from main player to dummy")
 MACRO_CONFIG_INT(MRXDummyCopyAim, mrx_dummy_copy_aim, 0, 0, 1, CFGFLAG_CLIENT | CFGFLAG_SAVE, "Copy aim from main player to dummy")
 MACRO_CONFIG_INT(MRXDummyCopyJump, mrx_dummy_copy_jump, 0, 0, 1, CFGFLAG_CLIENT | CFGFLAG_SAVE, "Copy jump from main player to dummy")
 MACRO_CONFIG_INT(MRXDummyCopyFire, mrx_dummy_copy_fire, 0, 0, 1, CFGFLAG_CLIENT | CFGFLAG_SAVE, "Copy fire from main player to dummy")
 MACRO_CONFIG_INT(MRXDummyCopyWeapon, mrx_dummy_copy_weapon, 0, 0, 1, CFGFLAG_CLIENT | CFGFLAG_SAVE, "Copy weapon from main player to dummy")
 MACRO_CONFIG_INT(MRXDummyCopyPlayerFlags, mrx_dummy_copy_playerflags, 0, 0, 1, CFGFLAG_CLIENT | CFGFLAG_SAVE, "Copy player flags from main player to dummy")
 MACRO_CONFIG_INT(MRXReverseDummyCopyMoves, mrx_reverse_dummy_copy_moves, 0, 0, 1, CFGFLAG_CLIENT | CFGFLAG_SAVE, "Reverse dummy copy moves")
 
 //MRX: Auto triple fly
 MACRO_CONFIG_INT(MRXTripleHookHolder, mrx_triplehookholder, 0, 0, 1, CFGFLAG_CLIENT | CFGFLAG_SAVE, "Enable Triple Hook Holder for dummy")
 MACRO_CONFIG_INT(MRXAutotriplefly, mrx_autotriplefly, 0, 0, 1, CFGFLAG_CLIENT | CFGFLAG_SAVE, "Enable auto triple fly (dummy holds player at fixed distance after hook hit)")
 MACRO_CONFIG_INT(MRXAutotripleflyMode, mrx_autotriplefly_mode, 1, 0, 1, CFGFLAG_CLIENT | CFGFLAG_SAVE, "Change TripleFly mode (0 = auto, 1 = manual)")
 MACRO_CONFIG_INT(MRXAutotripleflyDistance, mrx_autotriplefly_distance, 120, 10, 380, CFGFLAG_CLIENT | CFGFLAG_SAVE, "Distance for auto triple fly (manual mode)")
 MACRO_CONFIG_INT(MRXAutotripleflyManual, mrx_autotriplefly_manual, 0, 0, 1, CFGFLAG_CLIENT | CFGFLAG_SAVE, "Use manual distance for auto triple fly")
 MACRO_CONFIG_INT(MRXAutotripleflyThrow, mrx_autotriplefly_throw, 0, 0, 1, CFGFLAG_CLIENT | CFGFLAG_SAVE, "Enable auto triple fly throw (dummy hooks player near cursor and throws them)")
 MACRO_CONFIG_INT(MRXAutotripleflyThrowBoth, mrx_autotriplefly_throw_both, 0, 0, 1, CFGFLAG_CLIENT | CFGFLAG_SAVE, "Enable auto triple fly throw both (dummy and player attack together)")
 MACRO_CONFIG_INT(MRXAutotripleflyCooldown, mrx_autotriplefly_cooldown, 50, 0, 200, CFGFLAG_CLIENT | CFGFLAG_SAVE, "Cooldown in ticks after attack before dummy can hook again")


 
 // TripleFly state tracking
 MACRO_CONFIG_INT(MRXTripleFlyFunState, mrx_triplefly_fun_state, 0, 0, 1, CFGFLAG_CLIENT | CFGFLAG_SAVE, "TripleFlyFun active state")
 MACRO_CONFIG_INT(MRXTripleFlyNormalState, mrx_triplefly_normal_state, 0, 0, 1, CFGFLAG_CLIENT | CFGFLAG_SAVE, "TripleFlyNormal active state")
 
 // Hidden MRX menu toggle
 MACRO_CONFIG_INT(ClPlayerDeffis, cl_player_deffis, 0, 0, 1, CFGFLAG_CLIENT | CFGFLAG_SAVE, "Show player deffis")
 // Hidden console command
 MACRO_CONFIG_INT(ClPlayerWarn, cl_player_warn, 0, 0, 1, CFGFLAG_CLIENT | CFGFLAG_SAVE, "Enable console suggestions and auto-completion")
 
 MACRO_CONFIG_COL(MRXFriendColor, mrx_friend_color, 0xFFFFFFFF, CFGFLAG_CLIENT | CFGFLAG_SAVE, "Friend color")
 MACRO_CONFIG_COL(MRXFoeColor, mrx_foe_color, 0xFFFFFFFF, CFGFLAG_CLIENT | CFGFLAG_SAVE, "Foe color")
 MACRO_CONFIG_INT(MRXScoreboard, mrx_scoreboard, 0, 0, 1, CFGFLAG_CLIENT | CFGFLAG_SAVE, "Enable MRX scoreboard")
MACRO_CONFIG_INT(MRXHideChatEmote, mrx_hide_chat_emote, 0, 0, 1, CFGFLAG_CLIENT | CFGFLAG_SAVE, "Hide chat emote state from server")
 
 // MRX: Background lines settings for menus sections
 MACRO_CONFIG_INT(MRXBgLinesEnable, mrx_bglines_enable, 1, 0, 1, CFGFLAG_CLIENT | CFGFLAG_SAVE, "Enable animated background lines in MRX settings sections")
 MACRO_CONFIG_INT(MRXBgLinesDots, mrx_bglines_dots, 1, 0, 1, CFGFLAG_CLIENT | CFGFLAG_SAVE, "Draw dots at points for background lines")
 MACRO_CONFIG_INT(MRXBgLinesPoints, mrx_bglines_points, 18, 3, 256, CFGFLAG_CLIENT | CFGFLAG_SAVE, "Number of moving points for background lines per section")
 MACRO_CONFIG_INT(MRXBgLinesDistance, mrx_bglines_distance, 180, 1, 500, CFGFLAG_CLIENT | CFGFLAG_SAVE, "Max distance to connect two points with a line in pixels")
 MACRO_CONFIG_INT(MRXBgLinesSpeed, mrx_bglines_speed, 50, 1, 200, CFGFLAG_CLIENT | CFGFLAG_SAVE, "Animation speed of background lines (percent)")
 MACRO_CONFIG_INT(MRXBgLinesRadius, mrx_bglines_radius, 2, 1, 20, CFGFLAG_CLIENT | CFGFLAG_SAVE, "Dot radius for background lines (pixels)")
 MACRO_CONFIG_INT(MRXBgLinesAlpha, mrx_bglines_alpha, 18, 0, 100, CFGFLAG_CLIENT | CFGFLAG_SAVE, "Base alpha for background lines (percent)")
 MACRO_CONFIG_COL(MRXBgLinesColor, mrx_bglines_color, 0xFFFFFFFF, CFGFLAG_CLIENT | CFGFLAG_SAVE | CFGFLAG_COLALPHA, "Color of background lines and dots")
 
 // MRX Duo pseudo
 MACRO_CONFIG_INT(MRXDuoPseudo, mrx_duopseudo, 0, 0, 1, CFGFLAG_CLIENT | CFGFLAG_SAVE, "Enable duopseudo")
 MACRO_CONFIG_INT(MRXDuoPseudoTarget, mrx_duopseudotarget, 0, 0, 1, CFGFLAG_CLIENT | CFGFLAG_SAVE, "Target duopseudo")
 MACRO_CONFIG_INT(MRXDuoPseudoAutoAim, mrx_duopseudoautoaim, 0, 0, 1, CFGFLAG_CLIENT | CFGFLAG_SAVE, "Autoaim duopseudo")
 MACRO_CONFIG_INT(MRXDuoPseudoFire, mrx_duopseudofire, 0, 0, 1, CFGFLAG_CLIENT | CFGFLAG_SAVE, "Enabel hammerhit only 63")

 //Spof Version
MACRO_CONFIG_INT(MRXSpoofedVersion, mrx_spoofversion, 0, 0, 1, CFGFLAG_CLIENT | CFGFLAG_SAVE, "Enable MRX spoof version")
MACRO_CONFIG_INT(MRXSpoofedVersionNumber, mrx_spoofversionnumber, 0, 0, 1000000, CFGFLAG_CLIENT | CFGFLAG_SAVE, "Spoofed numeric DDNet version")
MACRO_CONFIG_STR(MRXSpoofedVersionString, mrx_spoofversionstring, 64, "", CFGFLAG_CLIENT | CFGFLAG_SAVE, "Spoofed human-readable version string")
MACRO_CONFIG_STR(MRXSpoofedNetVersion7, mrx_spoofednetversion7, 128, "", CFGFLAG_CLIENT | CFGFLAG_SAVE, "Spoofed 0.7 network version string")
MACRO_CONFIG_STR(MRXSpoofedNetVersion, mrx_spoofednetversion, 128, "", CFGFLAG_CLIENT | CFGFLAG_SAVE, "Spoofed 0.6 network version string")
MACRO_CONFIG_INT(MRXSpoofedClientVersion, mrx_spoofedclientversion, 0, 0, 0xFFFF, CFGFLAG_CLIENT | CFGFLAG_SAVE, "Spoofed 0.7 client version (decimal of hex)")

// MRX Aimbot (new implementation)
MACRO_CONFIG_INT(MRXAimbotEnable, mrx_aimbot_enable, 0, 0, 1, CFGFLAG_SAVE | CFGFLAG_CLIENT, "Enable MRX aimbot")
MACRO_CONFIG_INT(MRXAimbotFov, mrx_aimbot_fov, 360, 0, 360, CFGFLAG_SAVE | CFGFLAG_CLIENT, "Aimbot field of view in degrees")
MACRO_CONFIG_INT(MRXAimbotSilent, mrx_aimbot_silent, 0, 0, 1, CFGFLAG_SAVE | CFGFLAG_CLIENT, "Enable silent aim")
MACRO_CONFIG_INT(MRXAimbotAutoHook, mrx_aimbot_autohook, 0, 0, 1, CFGFLAG_SAVE | CFGFLAG_CLIENT, "Enable auto hook")
MACRO_CONFIG_INT(MRXAimbotEdge, mrx_aimbot_edge, 0, 0, 1, CFGFLAG_SAVE | CFGFLAG_CLIENT, "Enable edge scan fallback")
MACRO_CONFIG_INT(MRXAimbotScanType, mrx_aimbot_scan_type, 0, 0, 3, CFGFLAG_SAVE | CFGFLAG_CLIENT, "Scan type selector")
MACRO_CONFIG_INT(MRXAimbotAccuracy, mrx_aimbot_accuracy, 100, 0, 100, CFGFLAG_SAVE | CFGFLAG_CLIENT, "Aimbot accuracy percent")
MACRO_CONFIG_INT(MRXAimbotAutoHit, mrx_aimbot_autohit, 0, 0, 1, CFGFLAG_SAVE | CFGFLAG_CLIENT, "Enable auto hammer hit in aimbot");

// MRX ESP (new implementation)
MACRO_CONFIG_INT(MRXEspEnable, mrx_esp_enable, 0, 0, 1, CFGFLAG_SAVE | CFGFLAG_CLIENT, "Enable MRX ESP")
MACRO_CONFIG_INT(MRXEspDrawFov, mrx_esp_draw_fov, 0, 0, 1, CFGFLAG_SAVE | CFGFLAG_CLIENT, "Draw aimbot FOV helper")
MACRO_CONFIG_INT(MRXEspDrawBox, mrx_esp_draw_box, 1, 0, 1, CFGFLAG_SAVE | CFGFLAG_CLIENT, "Draw ESP boxes around players")
MACRO_CONFIG_INT(MRXEspGlowPlayer, mrx_esp_glow_player, 0, 0, 1, CFGFLAG_SAVE | CFGFLAG_CLIENT, "Draw ESP glow around players")

// MRX Wheel Binds
MACRO_CONFIG_INT(MRXBindWheelKey, mrx_bindwheel_key, 0, 0, 0xFFFF, CFGFLAG_CLIENT | CFGFLAG_SAVE, "Key for bind wheel")
MACRO_CONFIG_INT(MRXBindWheelResetMouse, mrx_bindwheel_reset_mouse, 0, 0, 1, CFGFLAG_CLIENT | CFGFLAG_SAVE, "Reset mouse position when opening bind wheel")

//MRX Custom Background Images
MACRO_CONFIG_INT(MRXCustomBgEnable, mrx_custombg_enable, 0, 0, 1, CFGFLAG_CLIENT | CFGFLAG_SAVE, "Enable custom background image")
MACRO_CONFIG_STR(MRXCustomBgPath, mrx_custombg_path, 256, "", CFGFLAG_CLIENT | CFGFLAG_SAVE, "Path to custom background image")
MACRO_CONFIG_INT(MRXCustomBgAlpha, mrx_custombg_alpha, 100, 0, 100, CFGFLAG_CLIENT | CFGFLAG_SAVE, "Custom background image alpha (0-100)")
MACRO_CONFIG_INT(MRXCustomBgBlur, mrx_custombg_blur, 0, 0, 100, CFGFLAG_CLIENT | CFGFLAG_SAVE, "Custom background blur amount (0-100)")
MACRO_CONFIG_INT(MRXCustomBgBlurCenter, mrx_custombg_blur_center, 0, 0, 100, CFGFLAG_CLIENT | CFGFLAG_SAVE, "Custom background center-focused blur (0-100)")

//MRX Auto Dummy Connect
MACRO_CONFIG_INT(MRXAutoDummyConnect, mrx_auto_dummy_connect, 0, 0, 1, CFGFLAG_CLIENT | CFGFLAG_SAVE, "Enable auto dummy connect when joining server")
MACRO_CONFIG_INT(MRXAutoDummyConnectCheckInterval, mrx_auto_dummy_connect_check_interval, 1000, 500, 10000, CFGFLAG_CLIENT | CFGFLAG_SAVE, "Check interval for dummy connect in milliseconds")

//MRX Auto Team
MACRO_CONFIG_INT(MRXAutoTeamEnable, mrx_auto_team_enable, 0, 0, 1, CFGFLAG_CLIENT | CFGFLAG_SAVE, "Enable auto team join")
MACRO_CONFIG_INT(MRXAutoTeamNumber, mrx_auto_team_number, 1, 1, 63, CFGFLAG_CLIENT | CFGFLAG_SAVE, "Team number to join automatically")
MACRO_CONFIG_INT(MRXAutoTeamInviteDummy, mrx_auto_team_invite_dummy, 0, 0, 1, CFGFLAG_CLIENT | CFGFLAG_SAVE, "Invite dummy to team")
MACRO_CONFIG_INT(MRXAutoTeamMode, mrx_auto_team_mode, 0, 0, 1, CFGFLAG_CLIENT | CFGFLAG_SAVE, "Auto team mode: 0=auto on join, 1=by bind")
MACRO_CONFIG_INT(MRXAutoTeamBind, mrx_auto_team_bind, 0, 0, 0xFFFF, CFGFLAG_CLIENT | CFGFLAG_SAVE, "Key bind for auto team")
MACRO_CONFIG_INT(MRXAutoTeamExecute, mrx_auto_team_execute, 0, 0, 1, CFGFLAG_CLIENT | CFGFLAG_SAVE, "Execute auto team join")

// MRX TAS (Tool-Assisted Speedrun)
// Основные элементы управления
MACRO_CONFIG_INT(MRXTasTps, mrx_tastps, 50, 1, 200, CFGFLAG_CLIENT | CFGFLAG_SAVE, "TAS ticks per second (simulation speed)")
MACRO_CONFIG_INT(MRXTasPause, mrx_taspause, 0, 0, 1, CFGFLAG_CLIENT | CFGFLAG_SAVE, "TAS pause state")
MACRO_CONFIG_INT(MRXLoadReplay, mrx_loadreplay, 0, 0, 1, CFGFLAG_CLIENT | CFGFLAG_SAVE, "Load and play replay")
MACRO_CONFIG_INT(MRXRecordReplay, mrx_recordreplay, 0, 0, 1, CFGFLAG_CLIENT | CFGFLAG_SAVE, "Record replay")
MACRO_CONFIG_INT(MRXTasRespawn, mrx_tasrespawn, 0, 0, 1, CFGFLAG_CLIENT | CFGFLAG_SAVE, "Clear replay and respawn")
MACRO_CONFIG_INT(MRXTasRewind, mrx_tasrewind, 0, 0, 1, CFGFLAG_CLIENT | CFGFLAG_SAVE, "Rewind replay")
MACRO_CONFIG_INT(MRXTasForward, mrx_tasforward, 0, 0, 1, CFGFLAG_CLIENT | CFGFLAG_SAVE, "Forward through replay")

// Настройки TAS
MACRO_CONFIG_INT(MRXTasUseSound, mrx_tasusesound, 0, 0, 1, CFGFLAG_CLIENT | CFGFLAG_SAVE, "Enable sound in TAS world simulation")
MACRO_CONFIG_INT(MRXTasShowEffects, mrx_tasshoweffects, 0, 0, 1, CFGFLAG_CLIENT | CFGFLAG_SAVE, "Enable visual effects in TAS world simulation")
MACRO_CONFIG_INT(MRXTasShowAim, mrx_tasshowaim, 0, 0, 1, CFGFLAG_CLIENT | CFGFLAG_SAVE, "Show real aim during replay playback")
MACRO_CONFIG_INT(MRXTasPlayerPrediction, mrx_tasplayerprediction, 0, 0, 1, CFGFLAG_CLIENT | CFGFLAG_SAVE, "Enable other players in TAS world simulation")

// Управление воспроизведением
MACRO_CONFIG_INT(MRXTasReplayVaultAutoSync, mrx_tasreplayvaultautosync, 0, 0, 1, CFGFLAG_CLIENT | CFGFLAG_SAVE, "Auto-sync with replay vault on startup")

// Инструменты TAS
MACRO_CONFIG_INT(MRXTasTickControlTicks, mrx_tastickcontrolticks, 1, 1, 100, CFGFLAG_CLIENT | CFGFLAG_SAVE, "Number of ticks to step forward/backward")
MACRO_CONFIG_INT(MRXTasAutoRewind, mrx_tasautorewind, 0, 0, 1, CFGFLAG_CLIENT | CFGFLAG_SAVE, "Auto-rewind before freeze state")
MACRO_CONFIG_INT(MRXTasAutoForward, mrx_tasautoforward, 0, 0, 1, CFGFLAG_CLIENT | CFGFLAG_SAVE, "Auto-forward through freeze state")
MACRO_CONFIG_INT(MRXTickControlPause, mrx_tickcontrolpause, 0, 0, 1, CFGFLAG_CLIENT | CFGFLAG_SAVE, "Auto-pause after auto-rewind/forward")
MACRO_CONFIG_INT(MRXTickControlStep, mrx_tickcontrolstep, 0, 0, 1, CFGFLAG_CLIENT | CFGFLAG_SAVE, "Step mode: single tick per press")

// Визуальные эффекты TAS
MACRO_CONFIG_INT(MRXTasDrawStartEndPos, mrx_tasdrawstartendpos, 0, 0, 1, CFGFLAG_CLIENT | CFGFLAG_SAVE, "Draw start/end position markers")
MACRO_CONFIG_INT(MRXTasDrawPath, mrx_tasdrawpath, 0, 0, 1, CFGFLAG_CLIENT | CFGFLAG_SAVE, "Draw replay path")
MACRO_CONFIG_INT(MRXTasDrawPathSegmented, mrx_tasdrawpathsegmented, 0, 0, 1, CFGFLAG_CLIENT | CFGFLAG_SAVE, "Draw only path segment near current position")
MACRO_CONFIG_INT(MRXTasDrawPathMode, mrx_tasdrawpathmode, 0, 0, 1, CFGFLAG_CLIENT | CFGFLAG_SAVE, "Path draw mode (0=dashed, 1=solid)")
MACRO_CONFIG_COL(MRXTasDrawPathColor, mrx_tasdrawpathcolor, 0xFFFFFFFF, CFGFLAG_CLIENT | CFGFLAG_SAVE, "Replay path color")
MACRO_CONFIG_INT(MRXTasDrawPredictionPath, mrx_tasdrawpredictionpath, 0, 0, 1, CFGFLAG_CLIENT | CFGFLAG_SAVE, "Draw prediction path during recording")
MACRO_CONFIG_INT(MRXTasDrawPredictionPathMode, mrx_tasdrawpredictionpathmode, 0, 0, 1, CFGFLAG_CLIENT | CFGFLAG_SAVE, "Prediction path mode (0=dashed, 1=solid)")
MACRO_CONFIG_COL(MRXTasDrawPredictionPathColor, mrx_tasdrawpredictionpathcolor, 0x00FF00FF, CFGFLAG_CLIENT | CFGFLAG_SAVE, "Prediction path color (local player/dummy)")
MACRO_CONFIG_COL(MRXTasDrawPredictionPathColorFrozen, mrx_tasdrawpredictionpathcolorfrozen, 0x0000FFFF, CFGFLAG_CLIENT | CFGFLAG_SAVE, "Prediction path color when frozen")
MACRO_CONFIG_COL(MRXTasDrawPredictionPathColorOthers, mrx_tasdrawpredictionpathcolorothers, 0xFFFF00FF, CFGFLAG_CLIENT | CFGFLAG_SAVE, "Prediction path color (other players)")