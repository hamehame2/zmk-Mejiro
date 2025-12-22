zephyr_library()
zephyr_include_directories(include)

zephyr_library_sources_ifdef(CONFIG_ZMK_MEJIRO
  src/behaviors/mejiro_core.c
  src/behaviors/mejiro_send_roman.c
  src/behaviors/behavior_mejiro.c
  src/behaviors/mejiro_tables.c

  src/behaviors/behavior_naginata.c
  src/naginata_func.c
  src/nglist.c
  src/nglistarray.c
)
