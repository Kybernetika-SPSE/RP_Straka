# Software hodinek
Software hodinek píšeme s pomocí ESP-IDF protože přímo od výrobce čipu obsahuje knihovny pro všechny periferie procesoru.

GUI je napsané v LVGL, stále potřebuje dost optimalizace a hlavně je dost limitované výběrem procesoru kde jsme chybně šli pro maximální úsporu elektřiny a ne pro výkon.

Můj kód je driver pro [IOE](components/ioe_tca6408a/) a [GUI](components/gui/).
Na [mainu](main/PlajTime.c) jsme samozřejmě spolupracovali, stejně jako na [board](components/board/) a na driveru pro [AMOLED](components/amoled_co5300/).
