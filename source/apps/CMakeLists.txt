target_sources(${PRODUCT} PUBLIC
        ${CMAKE_CURRENT_LIST_DIR}/about_badge.c
        ${CMAKE_CURRENT_LIST_DIR}/asteroids.c
        ${CMAKE_CURRENT_LIST_DIR}/battlezone.c
        ${CMAKE_CURRENT_LIST_DIR}/clue.c
        ${CMAKE_CURRENT_LIST_DIR}/cube.c
        # ${CMAKE_CURRENT_LIST_DIR}/etch-a-sketch.c
        ${CMAKE_CURRENT_LIST_DIR}/game_of_life.c
        ${CMAKE_CURRENT_LIST_DIR}/ghost-detector.c
        ${CMAKE_CURRENT_LIST_DIR}/hacking_simulator.c
        ${CMAKE_CURRENT_LIST_DIR}/lunarlander.c
        # ${CMAKE_CURRENT_LIST_DIR}/magic-8-ball.c
        ${CMAKE_CURRENT_LIST_DIR}/maze.c
        ${CMAKE_CURRENT_LIST_DIR}/microban.c
        ${CMAKE_CURRENT_LIST_DIR}/pong.c
        ${CMAKE_CURRENT_LIST_DIR}/qc.c
        ${CMAKE_CURRENT_LIST_DIR}/smashout.c
        ${CMAKE_CURRENT_LIST_DIR}/simonSays.c
        # ${CMAKE_CURRENT_LIST_DIR}/tank-vs-tank.c
        ${CMAKE_CURRENT_LIST_DIR}/username.c
        ${CMAKE_CURRENT_LIST_DIR}/gulag.c
        ${CMAKE_CURRENT_LIST_DIR}/rvasec_splash.c
        ${CMAKE_CURRENT_LIST_DIR}/slot_machine.c
        ${CMAKE_CURRENT_LIST_DIR}/test-screensavers.c
        ${CMAKE_CURRENT_LIST_DIR}/moon-patrol.c
        ${CMAKE_CURRENT_LIST_DIR}/badgey.c
        ${CMAKE_CURRENT_LIST_DIR}/image-test.c
	${CMAKE_CURRENT_LIST_DIR}/2048.c
	${CMAKE_CURRENT_LIST_DIR}/puzzle-attack.c
	${CMAKE_CURRENT_LIST_DIR}/badgemon.c
	${CMAKE_CURRENT_LIST_DIR}/badgemon_unlock.c
	${CMAKE_CURRENT_LIST_DIR}/mixtape.c
        # ${CMAKE_CURRENT_LIST_DIR}/badge-app-template.c
	${CMAKE_CURRENT_LIST_DIR}/aagunner.c
	${CMAKE_CURRENT_LIST_DIR}/default_menu_app.c
	# ${CMAKE_CURRENT_LIST_DIR}/carousel_menu_app.c
	${CMAKE_CURRENT_LIST_DIR}/menu_2025.c
	${CMAKE_CURRENT_LIST_DIR}/screensaver_app.c
	${CMAKE_CURRENT_LIST_DIR}/drum_machine.c
        )
add_subdirectory(simonSays_assets)
add_subdirectory(clue_assets)
add_subdirectory(new_badge_monsters_assets)
add_subdirectory(new_badge_monsters)
target_include_directories(${PRODUCT} PUBLIC .)

target_add_png_badge_asset(${PRODUCT} 4 ${CMAKE_CURRENT_LIST_DIR}/badge_monster_assets/badge_monster_birdo.png)
target_add_png_badge_asset(${PRODUCT} 4 ${CMAKE_CURRENT_LIST_DIR}/badge_monster_assets/badge_monster_bowser.png)
target_add_png_badge_asset(${PRODUCT} 4 ${CMAKE_CURRENT_LIST_DIR}/badge_monster_assets/badge_monster_crawler.png)
target_add_png_badge_asset(${PRODUCT} 4 ${CMAKE_CURRENT_LIST_DIR}/badge_monster_assets/badge_monster_donkey_kong.png)
target_add_png_badge_asset(${PRODUCT} 4 ${CMAKE_CURRENT_LIST_DIR}/badge_monster_assets/badge_monster_ghosts.png)
target_add_png_badge_asset(${PRODUCT} 4 ${CMAKE_CURRENT_LIST_DIR}/badge_monster_assets/badge_monster_harpy.png)
target_add_png_badge_asset(${PRODUCT} 4 ${CMAKE_CURRENT_LIST_DIR}/badge_monster_assets/badge_monster_jason_voorhees.png)
target_add_png_badge_asset(${PRODUCT} 4 ${CMAKE_CURRENT_LIST_DIR}/badge_monster_assets/badge_monster_medusa_head.png)
target_add_png_badge_asset(${PRODUCT} 4 ${CMAKE_CURRENT_LIST_DIR}/badge_monster_assets/badge_monster_metall.png)
target_add_png_badge_asset(${PRODUCT} 4 ${CMAKE_CURRENT_LIST_DIR}/badge_monster_assets/badge_monster_moblin.png)
target_add_png_badge_asset(${PRODUCT} 4 ${CMAKE_CURRENT_LIST_DIR}/badge_monster_assets/badge_monster_mother_brain.png)
target_add_png_badge_asset(${PRODUCT} 4 ${CMAKE_CURRENT_LIST_DIR}/badge_monster_assets/badge_monster_nettler.png)
target_add_png_badge_asset(${PRODUCT} 4 ${CMAKE_CURRENT_LIST_DIR}/badge_monster_assets/badge_monster_odd_eye.png)
target_add_png_badge_asset(${PRODUCT} 4 ${CMAKE_CURRENT_LIST_DIR}/badge_monster_assets/badge_monster_shredder.png)
target_add_png_badge_asset(${PRODUCT} 4 ${CMAKE_CURRENT_LIST_DIR}/badge_monster_assets/badge_monster_slime.png)
target_add_png_badge_asset(${PRODUCT} 4 ${CMAKE_CURRENT_LIST_DIR}/badge_monster_assets/badge_monster_stay_puff.png)

target_add_png_badge_asset(${PRODUCT} 4 ${CMAKE_CURRENT_LIST_DIR}/badge_monster_assets/badge_monster_beetlejuice.png)
target_add_png_badge_asset(${PRODUCT} 4 ${CMAKE_CURRENT_LIST_DIR}/badge_monster_assets/badge_monster_chet.png)
target_add_png_badge_asset(${PRODUCT} 4 ${CMAKE_CURRENT_LIST_DIR}/badge_monster_assets/badge_monster_chucky.png)
target_add_png_badge_asset(${PRODUCT} 4 ${CMAKE_CURRENT_LIST_DIR}/badge_monster_assets/badge_monster_drago.png)
target_add_png_badge_asset(${PRODUCT} 4 ${CMAKE_CURRENT_LIST_DIR}/badge_monster_assets/badge_monster_ed_rooney.png)
target_add_png_badge_asset(${PRODUCT} 4 ${CMAKE_CURRENT_LIST_DIR}/badge_monster_assets/badge_monster_freddy_krueger.png)
target_add_png_badge_asset(${PRODUCT} 4 ${CMAKE_CURRENT_LIST_DIR}/badge_monster_assets/badge_monster_gopher_caddyshack.png)
target_add_png_badge_asset(${PRODUCT} 4 ${CMAKE_CURRENT_LIST_DIR}/badge_monster_assets/badge_monster_gremlin.png)
target_add_png_badge_asset(${PRODUCT} 4 ${CMAKE_CURRENT_LIST_DIR}/badge_monster_assets/badge_monster_hans_gruber.png)
target_add_png_badge_asset(${PRODUCT} 4 ${CMAKE_CURRENT_LIST_DIR}/badge_monster_assets/badge_monster_jack_torrance.png)
target_add_png_badge_asset(${PRODUCT} 4 ${CMAKE_CURRENT_LIST_DIR}/badge_monster_assets/badge_monster_jason.png)
target_add_png_badge_asset(${PRODUCT} 4 ${CMAKE_CURRENT_LIST_DIR}/badge_monster_assets/badge_monster_joker.png)
target_add_png_badge_asset(${PRODUCT} 4 ${CMAKE_CURRENT_LIST_DIR}/badge_monster_assets/badge_monster_khan.png)
target_add_png_badge_asset(${PRODUCT} 4 ${CMAKE_CURRENT_LIST_DIR}/badge_monster_assets/badge_monster_richard_vernon.png)
target_add_png_badge_asset(${PRODUCT} 4 ${CMAKE_CURRENT_LIST_DIR}/badge_monster_assets/badge_monster_skeltor.png)
target_add_png_badge_asset(${PRODUCT} 4 ${CMAKE_CURRENT_LIST_DIR}/badge_monster_assets/badge_monster_stay_puft.png)
