# Аудит MITAB перед публикацией

Дата: 11 ноября 2026. Ветка `fix/windows-xp-build`, HEAD `c63ea893c2606827940319838d31ec0b2c7d0756`. Хост проверки: Windows 10.0.19045, 64-битный PowerShell. GDAL читался только в `C:\Users\grig\MITAB-Analysis\repositories\gdal`. Исходники, проекты Visual Studio, имена DLL и Delphi-фреймворк не менялись. Коммит и `git push` не создавались.

Рабочее дерево на начало аудита содержало один неотслеживаемый файл: `mitab.code-workspace`. Отслеживаемых `*.dll`, `*.exe`, `*.obj`, `*.lib` нет (`git ls-files`).

Этот файл фиксирует то, что проверено командой, дампом или чтением исходника. Там, где запуска на Windows XP или сборки Delphi не было, это записано отдельно.

## 1. Краткое резюме

Публиковать исходное дерево как продолжение MITAB можно после добавления корневого `LICENSE` и `NOTICE`. Основной текст лицензии в файлах — разрешение в стиле MIT/X. Два компонента внутри сборки живут на других условиях: `ogr/swq.c` (Information Interoperability Institute) и `cpl/cpl_recode_stub.cpp` (код FLTK, Bill Spitzak). Оба попадают в `mitab.dll`. `README.md` сейчас описывает только лицензию из `README.TXT`.

`mitab.dll` экспортирует 1770 имён. Из них 93 — публичный C API, остальные — следствие `CPL_DLL` = `__declspec(dllexport)`. Сократить список до этих 93 можно, не меняя сигнатуры: собирать CPL, OGR и MITAB с `CPL_DISABLE_DLL` и оставить `MITAB_DLL` только на `mitab_c_*`. Файл `.def` для этого не обязателен.

Переименование в `MITAB_NG_x86.dll` и `MITAB_NG_x64.dll` — это `TargetName` в `MITAB.vcxproj` и `OUTPUT_NAME` в `CMakeLists.txt`. Утилиты импортную библиотеку DLL не линкуют, поэтому сборка утилит от нового имени не зависит. Имена экспортов от имени файла не зависят. `LoadLibrary` / `GetProcAddress` на текущих DLL это подтвердили: Win32 отвечает на `_mitab_c_getlibversion@0`, x64 — на `mitab_c_getlibversion`. Обе возвращают `2000000`.

Win32-образ имеет subsystem 5.01 и зависит только от `KERNEL32.dll`. x64-образ имеет subsystem 6.00: в проектах для x64 записан номер 5.01, линкер его не принимает. Запуска на Windows XP не было.

Расхождение `10 20` и `9.99 19.995` — квантование целочисленных координат TAB при разных границах. `tab2tab` оставляет масштаб по умолчанию 1000. `ogr2ogr` ставит границы `-30000000,-15000000,30000000,15000000`. Преобразование Mollweide здесь ни при чём. Алгоритм `Coordsys2Int` менять не нужно.

`contrib/mitab_dyn.pas` объявляет и загружает все 93 функции, и все 93 размера стека совпадают с экспортами Win32. Для x64 этот модуль не годится: он ищет только декорированные имена и хранит `void*` в `Longword`. Сборка Delphi в этом аудите не запускалась.

## 2. Проблемы

| Приоритет | Тема | Суть |
|---|---|---|
| HIGH | Лицензия бинарной поставки | Нет корневого `LICENSE`. `README.md` называет одну MIT-лицензию. В DLL есть код 3i и код из FLTK, их уведомления в документацию поставки не вынесены. |
| HIGH | Лишние экспорты | 1770 экспортов вместо 93. Тот же набор уходит и в EXE, потому что `dllexport` стоит на объектных файлах. |
| HIGH | x64 и Windows XP | Образ x64 помечен subsystem 6.00. Для цели «XP x64» это формально чужой образ. Запуск на XP не выполнялся и для Win32. |
| HIGH | `contrib/mitab_dyn.pas` на x64 и на Unicode Delphi | Поиск только `_name@N`, хэндлы `Longword`, аргументы `pchar`. Модуль не компилировался. |
| MEDIUM | Copyright Even Rouault | В GDAL он уже стоит в трёх файлах, из которых взяты правки. В нашем дереве этих строк нет. Текст разрешения тот же. |
| MEDIUM | `contrib/dotnet/MiApi.cs`, `MiWrapper.cs` | Разрешения на использование в файлах нет. В DLL они не компилируются. |
| MEDIUM | `ogr/ogr_srs_esri_names.h` | Заголовок без copyright, файл входит в сборку OGR через include. |
| MEDIUM | Документы расходятся с кодом | `docs/DEVELOPMENT_REPORT.md` пишет, что `mitab_dyn.pas` не загружает коллекцию, проекцию, датум и affine. Сейчас загружает все 93. Комментарий в `MITAB.Utility.props` говорит, что DLL экспортирует только C API. |
| MEDIUM | `.gitignore` | Нет `*.exe` и `mitab.code-workspace`. `*.dll` и `*.lib` игнорируются. |
| LOW | Абсолютный путь | В отслеживаемом `docs/DEVELOPMENT_REPORT.md` записан локальный путь к репозиторию. Других таких путей в Git нет. |
| LOW | `mitab/allproj.cpp`, `cpl/cpl_config.h` | Нет уведомления о правах. `allproj.cpp` не входит в `MITAB.sln`. |
| INFO | Координаты | Расхождение `tab2tab` и `ogr2ogr` воспроизведено и объясняется границами TAB. Это не повод менять формулы. |

Критического разрыва лицензий, при котором код GDAL нельзя было бы сочетать с текущим деревом, не найдено. Восемь перенесённых коммитов не меняют текст разрешения.

## 3. Лицензии и авторские права

### 3.1. Основной текст MITAB

`README.TXT`, строки 17–52: библиотека и поддерживающие CPL/OGR распространяются с разрешением «Permission is hereby granted…», copyright Daniel Morissette (1998–2005), Frank Warmerdam (1998–2005), Stephane Villeneuve (1999, 2000). Условие: это уведомление сохраняется во всех копиях и существенных частях. Исходный код отдавать не требуется. Для бинарной копии уведомление тоже должно сопровождать поставку: формулировка относится ко всем копиям, не только к исходникам.

`README.md`, строки 61–63, отсылает только к этому тексту. Корневого `LICENSE` нет.

Тот же абзац разрешения стоит в файлах `mitab/`, `cpl/` и `ogr/`, которые перечислены в разделе 1 отчёта о разработке. Проверка обходом `*.c`, `*.cpp`, `*.h`, `*.pas`, `*.vb`, `*.cs` показала, что разрешение отсутствует у короткого списка ниже. Заголовки copyright в перенесённых файлах `mitab/` на месте: например `mitab/mitab_feature.cpp` по-прежнему начинается с copyright Daniel Morissette.

Список авторов в `README.TXT` короче, чем набор copyright в дереве. В файлах дополнительно есть Les Technologies SoftMap Inc. (`ogr/ogrspatialreference.cpp` и соседние), Andrey Kiselev (`cpl/cpl_strtod.cpp`), Even Rouault (`cpl/cpl_atomic_ops.cpp`, `cpl/cpl_vsil_stdout.cpp`), Bo Thomsen (`contrib/mitab_dyn.pas`). У этих файлов тот же MIT-абзац. Для поставки исходников достаточно сохранить заголовки. Для одной общей страницы лицензии их стоит перечислить в `NOTICE`, не подменяя текст в каждом файле.

### 3.2. Компоненты с другими условиями

**`ogr/swq.h` и `ogr/swq.c`, строки 8–17.** Copyright (C) 2001 Information Interoperability Institute (3i). Это не MIT-абзац. Условия в заголовке:

- можно использовать, копировать, изменять и распространять для любой цели без платы;
- copyright и этот текст разрешения должны быть во всех копиях;
- оба уведомления должны быть в сопроводительной документации;
- имя 3i нельзя использовать в рекламе без письменного разрешения.

`OGR.vcxproj`, строка 100, компилирует `ogr/swq.c`. Строка `Empty SQL request string` из `ogr/swq.c` есть в `bin\Win32\Release\mitab.dll`, `ogr2ogr.exe` и `tab2tab.exe`. В таблице экспортов имён `swq_` нет: функции не помечены `CPL_DLL`, но объект в образ попал. `ogr/ogrdatasource.cpp` ссылается на `swq_*`, а классы OGR входят в DLL.

Действие: в `NOTICE`, который лежит рядом с DLL и EXE, включить дословное уведомление из `ogr/swq.c`. Имя 3i в маркетинговых текстах не использовать.

**`cpl/cpl_recode_stub.cpp`, строки 8–28.** Комментарий говорит, что основная часть взята из `utf.c` FLTK (Bill Spitzak and others, 2006; Frank Warmerdam, 2008). Разрешение другое: «Permission to use, copy, modify, and distribute this software for any purpose… provided that the above copyright notice and this permission notice appear in all copies.» Отказ от гарантий сформулирован отдельно от MIT-абзаца. `CPL.vcxproj`, строка 94, компилирует этот файл. В экспортах Win32 есть `CPLRecode` (`exports-win32.txt`). Строк «FLTK» и «Bill Spitzak» внутри DLL нет: уведомление живёт только в исходнике.

Действие: то же уведомление включить в `NOTICE` бинарной поставки. Заголовок файла не переписывать.

**`contrib/dotnet/MiApi.cs`, `MiWrapper.cs`.** В начале только `$Id`. `contrib/dotnet/README_DOTNET.TXT` называет автора: Graham Sims, Environment Bay of Plenty. Разрешения нет. В `MITAB.vcxproj` этих файлов нет, в DLL они не входят. `README.md` нельзя читать так, будто MIT из `README.TXT` покрывает и эти два файла.

Действие: до публикации либо найти и записать разрешение, либо явно вынести каталог `contrib/dotnet` из заявления «вся поставка на условиях README.TXT». Файлы удалять на этом шаге не предлагается: они уже были в upstream.

**`ogr/ogr_srs_esri_names.h`.** Файл сразу начинается с `apszGcsNameMapping`. Copyright и разрешения нет. Это таблица строк, она компилируется в OGR через включающий `.cpp`. Отдельной лицензии ESRI или EPSG в дереве не найдено, файлов `*.csv` нет.

Действие: перед публикацией найти исходный заголовок этого файла в истории GDAL/MITAB. Если это текст Frank Warmerdam на том же разрешении, что `ogr/ogr_srs_esri.cpp`, поставить такой же заголовок. Пока происхождение заголовка не подтверждено, не записывать файл молча в общий MIT-список.

**`mitab/allproj.cpp`.** Нет copyright. В `MITAB.sln` его нет, в DLL он не входит.

**`cpl/cpl_config.h` и `cpl/cpl_config.h.vc`.** Локальный набор `HAVE_*` без copyright. На лицензию DLL не влияет. Имеет смысл короткий заголовок при следующем касании файла.

Сторонних библиотек в зависимостях DLL нет. `dumpbin /dependents` для `bin\Win32\Release\mitab.dll` и `bin\x64\Release\mitab.dll` показывает только `KERNEL32.dll`. PROJ, GEOS, curl, zlib, SQLite и CSV EPSG в дереве не поставляются. `importFromEPSG` без внешних CSV в этой сборке нечем питать; это ограничение функций, не отдельная лицензия.

### 3.3. Правки, перенесённые из GDAL

Восемь коммитов GDAL, авторы — Even Rouault. Ни один diff этих коммитов не содержит строк `Copyright` или `Permission is hereby granted` (`git show <sha> | Select-String`).

| SHA | Дата | Файлы GDAL |
|---|---|---|
| `a5cbb6c535` | 2012-05-02 | `mitab_coordsys.cpp`, `mitab_spatialref.cpp` |
| `2789405d5b` | 2018-08-02 | `mitab_feature.cpp` |
| `1a9fbce231` | 2014-08-15 | `mitab_feature_mif.cpp` |
| `bc34cd533c`, `588cc86485`, `62d8f0bb3d` | 2015-12 | `mitab_feature_mif.cpp` |
| `2c5c3db03f` | 2014-12-29 | `mitab_mapindexblock.cpp` |
| `6edba1e791` | 2015-12-31 | `mitab_mapobjectblock.cpp` |

Заголовок `mitab_coordsys.cpp` на `a5cbb6c535` — copyright Frank Warmerdam и тот же MIT-абзац, что в нашем файле. `mitab_feature_mif.cpp` на коммитах 2014–2015 — copyright Stephane Villeneuve, совпадает с нашим.

На коммитах-источниках в GDAL уже есть вторая строка copyright, которой у нас нет:

- `2789405d5b:mitab_feature.cpp` — `Copyright (c) 2014, Even Rouault <even.rouault at spatialys.com>`
- `2c5c3db03f:mitab_mapindexblock.cpp` — та же строка
- `6edba1e791:mitab_mapobjectblock.cpp` — та же строка

Сами diff этих трёх коммитов строку не добавляли: она уже была в файле GDAL. Текст разрешения в этих файлах тот же MIT-абзац. Отдельного `LICENSE` в дереве GDAL на `a5cbb6c535` нет. Текущий `LICENSE.TXT` GDAL описывает общее разрешение GDAL/OGR как MIT-style; это лицензия сегодняшнего GDAL, не текст тех коммитов. Для переноса опираться нужно на заголовки файлов на названных SHA. Они совпадают с нашим разрешением.

Действие: в трёх наших файлах добавить строку copyright Even Rouault, как в GDAL на SHA источника, и перечислить восемь SHA в `NOTICE`. Формулы и логику этих правок не трогать.

### 3.4. Какие файлы класть рядом с DLL и EXE

Рядом с `MITAB_NG_x86.dll`, `MITAB_NG_x64.dll` и утилитами:

1. `LICENSE` — дословный MIT-абзац из `README.TXT` (строки 30–52) и перечень copyright из заголовков, включая SoftMap, Kiselev, Rouault, Thomsen.
2. `NOTICE` — дословные тексты из `ogr/swq.c` (строки 8–17) и `cpl/cpl_recode_stub.cpp` (строки 14–28), плюс список перенесённых коммитов GDAL и их автор.
3. Не утверждать в `README.md`, что `contrib/dotnet` и `ogr_srs_esri_names.h` покрыты одним абзацем, пока для них нет такого текста.

Исходная поставка (git archive) уже содержит заголовки в файлах. `LICENSE` и `NOTICE` нужны и там, чтобы заявление в `README.md` не было шире файлов.

## 4. Экспорты DLL

### 4.1. Что в образе

`dumpbin /exports` (COFF/PE Dumper 12.00.40629.0), 11 ноября 2026:

| Образ | Функций | Имён | `mitab_c_*` |
|---|---:|---:|---|
| `bin\Win32\Release\mitab.dll` | 1770 | 1770 | 93, все вида `_mitab_c_<name>@<bytes>` |
| `bin\x64\Release\mitab.dll` | 1770 | 1770 | 93, все без декорации |

Примеры Win32: `_mitab_c_open@4`, `_mitab_c_set_affine_params@12`. На x64 тех же декорированных имён нет.

Те же 1770 имён экспортируют `tab2tab.exe` и `tabdump.exe` обеих платформ. `ogr2ogr.exe` экспортирует 1771. Утилиты линкуют объектные файлы MITAB (`MITAB.Utility.props`, target `LinkMitabObjects`) и не используют импортную библиотеку DLL (`LinkLibraryDependencies` = false в `tabdump.vcxproj`, строки 65–68). `dllexport` поэтому попадает в EXE из `.obj`, а не из импорта DLL.

### 4.2. Откуда берётся `CPL_DLL`

`cpl/cpl_port.h`, строки 231–233:

```c
#ifndef CPL_DLL
#if defined(_MSC_VER) && !defined(CPL_DISABLE_DLL)
#  define CPL_DLL     __declspec(dllexport)
```

`cpl/cpl_config.h`, строка 34: `#undef CPL_DISABLE_DLL`. Макрос остаётся неопределённым, и на MSVC `CPL_DLL` раскрывается в `dllexport`. Им помечены функции CPL и классы OGR (`ogr/ogrsf_frmts.h` и соседние заголовки).

Публичный C API использует другой макрос. `mitab/mitab_capi.h`, строки 172–175:

```c
#ifdef _WIN32
# ifndef MITAB_CDECL
#   define MITAB_DLL  __declspec(dllexport)
#   define MITAB_STDCALL __stdcall
```

На x64 MSVC принимает `__stdcall` и не декорирует имя. Это совпало с дампом.

`contrib/mitab.def` — включаемый файл MapBasic (комментарий в строках 1–9), а не linker `.def`. В `MITAB.vcxproj` нет `ModuleDefinitionFile`.

### 4.3. Как сократить список

Безопасный путь, без смены 93 сигнатур и без смены `@N`:

1. В препроцессор `CPL.vcxproj`, `OGR.vcxproj` и `MITAB.vcxproj` (все четыре конфигурации) добавить `CPL_DISABLE_DLL`. Тогда `CPL_DLL` в этих единицах трансляции станет пустым.
2. `MITAB_DLL` не переопределять. Объявления в `mitab_capi.h` останутся `__declspec(dllexport) __stdcall`.
3. Утилиты пересобрать теми же объектами. Их EXE перестанут экспортировать чужие символы заодно. На их запуск это не должно влиять: они и сейчас зовут классы из своих `.obj`, а не через импорт DLL.
4. После Rebuild сравнить `dumpbin /exports` с текущими 93 именами. Для Win32 набор должен совпасть с `_mitab_c_*@N` один в один. Для x64 — с недекорированными именами.

`.def` применим как второй замок, но он хуже первого шага. В Win32 в `EXPORTS` нужно писать уже декорированные `_mitab_c_open@4`. Обычное имя в `.def` может сменить экспорт и сломать `GetProcAddress` у Delphi. Для x64 список другой, без `@N`. Два `.def` легко разъедутся с заголовком. Пока хватает `CPL_DISABLE_DLL`, отдельный `.def` не заводить.

Что изменится для тех, кто уже линкует `mitab.lib` и зовёт классы C++ или `CPL*` из DLL: эти символы исчезнут. В дереве таких потребителей нет. Утилиты импорт не используют. Комментарий в `MITAB.Utility.props` (строки 4–6) уже говорит, что внешний контракт — stdcall C API; сам комментарий при этом ошибочно утверждает, что другие экспорты отсутствуют. После сокращения комментарий станет верным.

C++ ABI классов внутри DLL этот шаг не перекраивает: утилиты по-прежнему собираются из тех же `.obj`. Меняется только таблица экспорта. Глобальный `DEBUG` по-прежнему нельзя включать: он добавляет виртуальные `Dump` и меняет vtable (`CONTRIBUTING.md`).

Совместимость Win32 и x64: декорация разная, набор из 93 функций один. Сокращение экспортов нужно проверить отдельным `dumpbin` на обеих платформах. Один `.def` на обе платформы не подойдёт.

## 5. План имён `MITAB_NG_x86.dll` и `MITAB_NG_x64.dll`

Имя задаётся в одном месте проекта DLL.

| Место | Сейчас |
|---|---|
| `MITAB.vcxproj`, строка 56 | `<TargetName>mitab</TargetName>` на все платформы |
| `MITAB.vcxproj`, строки 53–54 | `OutDir` = `bin\$(Platform)\$(Configuration)\`. `TargetExt` и `OutputFile` не заданы, расширение остаётся `.dll` |
| `MITAB.vcxproj` | `ImportLibrary` не задан, импорт будет `$(OutDir)$(TargetName).lib` |
| `CMakeLists.txt`, строка 125 | `OUTPUT_NAME mitab` |
| `CPL.vcxproj` / `OGR.vcxproj` | `TargetName` `cpl` и `ogr`, это статические библиотеки, их не переименовывать |

Утилиты берут `*.obj` из `obj\MITAB\...` и `cpl.lib` / `ogr.lib`. Имя DLL в их командную строку линковки не входит. Сборка `MITAB.sln` после смены `TargetName` не должна потерять EXE.

Шаги, когда будет разрешение менять проекты:

1. В `MITAB.vcxproj` заменить общий `TargetName` на два условных: `Win32` → `MITAB_NG_x86`, `x64` → `MITAB_NG_x64`. `TargetExt` не задавать.
2. В `CMakeLists.txt` поставить тот же `OUTPUT_NAME` по `CMAKE_SIZEOF_VOID_P`. CMake в этот день не запускался; правка нужна, чтобы два генератора не разъехались.
3. Версию не менять: `MITAB_VERSION_INT` `2000000` в `mitab/mitab.h`, строки 126–127. Сигнатуры и `__stdcall` не менять.
4. Обновить строки `mitab.dll` в `README.md`, `CONTRIBUTING.md`, комментарии `MITAB.Utility.props`. В `contrib/mitab_dyn.pas` строка 543 задаёт `LibPath:= 'mitab.dll'`. В `contrib/mitab.vb` и `contrib/dotnet/MiApi.cs` имя DLL зашито в `Declare` / `DllImport`. Одно строковое имя не покрывает x86 и x64. В привязках оставить путь настраиваемым и записать в комментарии оба имени файла. Декорированные alias в `mitab.vb` (`_mitab_c_open@4` и остальные) не менять: они про экспорт, не про имя файла.
5. Внешний `D:\Delphi\csGis10\csMitabDyn.pas` не копировать и не править. Он выбирает DLL своими константами (`MitabFromGDAL_VS_11_x86.dll` и соседние, строки 123–132). После переименования путь к новой DLL задаётся там отдельно.

Проверка загрузки, уже сделанная на текущих именах:

- 32-битный PowerShell, `bin\Win32\Release\mitab.dll`: `GetProcAddress("_mitab_c_getlibversion@0")` нашёл адрес, `GetProcAddress("mitab_c_getlibversion")` не нашёл. Вызов вернул `2000000`.
- 64-битный PowerShell, `bin\x64\Release\mitab.dll`: наоборот, нашёл только `mitab_c_getlibversion` и вернул `2000000`. Тот же процесс получил от Win32 DLL ошибку `LoadLibrary` 193 (`ERROR_BAD_EXE_FORMAT`): 64-битный процесс не грузит PE32. Это ожидаемо и к имени файла не относится.

После переименования повторить эти два вызова уже с новыми путями. Таблица экспорта от `TargetName` не меняется.

## 6. Windows XP

Три уровня ниже не смешиваются.

### 6.1. Сборка

В `bin\Win32\Release` и `bin\x64\Release` лежат `mitab.dll`, `tab2tab.exe`, `ogr2ogr.exe`, `ogrinfo.exe`, `tabdump.exe`. Метка времени заголовка Win32 DLL: Wed Nov 11 14:23:29 2026. x64 DLL: Wed Nov 11 14:24:56 2026. Toolset в проектах — `v120_xp`. В этом аудите Rebuild не запускался; образы — результат уже записанной сборки 11 ноября. `Debug|Win32` и `Debug|x64` по `docs/DEVELOPMENT_REPORT.md` не собирались, и этот аудит их тоже не собирал.

### 6.2. Формальный PE

`dumpbin /headers`:

| Образ | Machine | Magic | OS / subsystem | Subsystem |
|---|---|---|---|---|
| Win32 `mitab.dll` | `14C` x86 | PE32 `10B` | 5.01 / 5.01 | 2, Windows GUI |
| x64 `mitab.dll` | `8664` | PE32+ `20B` | 6.00 / 6.00 | 2, Windows GUI |
| Win32 `tab2tab.exe`, `tabdump.exe`, `ogrinfo.exe`, `ogr2ogr.exe` | x86 | PE32 | 5.01 / 5.01 | 3, Windows CUI |
| x64 те же EXE | x64 | PE32+ | 6.00 / 6.00 | 3, Windows CUI |

Причина номера 6.00 записана в проектах и в прошлом журнале сборки. `MITAB.vcxproj`, строки 68–70, и `MITAB.Utility.props`, строки 18–20, для всех платформ задают `/SUBSYSTEM:WINDOWS,5.01` и `/SUBSYSTEM:CONSOLE,5.01`. `docs/DEVELOPMENT_REPORT.md` фиксирует, что линкер на x64 напечатал LNK4010 и подставил версию по умолчанию. Повторно линкер в этом аудите не запускался; заголовки образов с 6.00 этому описанию соответствуют.

`CMakeLists.txt`, строки 136–139, уже различает платформы: Win32 получает `5.01`, 64-bit получает `5.02`. Проекты Visual Studio этого разделения не делают. Номер 5.02 — то значение, которое CMake уже считает допустимым для 64-bit образа этого дерева. Пока vcxproj не изменены, x64-образ остаётся 6.00.

Зависимости обоих DLL: только `KERNEL32.dll`. CRT статический (`/MT` в `MITAB.vcxproj`, строки 64–65): `MSVCR120.dll` в dependents нет.

Имена импорта сверялись со списком функций, которые обычно отсутствуют до Windows Vista: `InitializeCriticalSectionEx`, `GetTickCount64`, `CompareStringEx`, `LCMapStringEx`, `GetLocaleInfoEx`, `GetFileInformationByHandleEx`, `InitializeConditionVariable`. Этих имён в `dumpbin /imports` обоих DLL нет. В обоих образах есть `EncodePointer`, `DecodePointer`, `InitializeCriticalSectionAndSpinCount`, `GetModuleHandleExW`, `FindFirstFileExW`, `SetFilePointerEx`. В x64 дополнительно есть `RtlLookupFunctionEntry`, `RtlUnwindEx`, `RtlCaptureContext`, `RtlVirtualUnwind`. Наличие имени в import table не доказывает, что оно экспортируется `kernel32.dll` именно Windows XP. Сверка с XP-шным `kernel32.dll` не делалась.

### 6.3. Запуск на Windows XP

Не выполнялся. Ни Win32-образ с пометкой 5.01, ни x64-образ с пометкой 6.00 на Windows XP не загружались. Успешная сборка и формальный subsystem это не заменяют.

Когда появится машина XP, минимум такой: скопировать Win32 DLL и `tab2tab.exe` на XP SP3, запустить `tab2tab` без аргументов и один MIF→TAB из раздела 7. Для XP x64 сначала получить образ с subsystem 5.02 и только потом повторить запуск. До этого писать «совместимо с XP» нельзя.

## 7. Точность координат

### 7.1. Воспроизведение

Каталог вне репозитория: `%TEMP%\mitab-audit-coords`. Вход `sample.mif`, ASCII без BOM:

```
CoordSys Earth Projection 13, 104, "m", 0
Point 10 20
```

Плюс `Region` с вершинами `0 0`, `10 0`, `10 10`, `0 10`, `0 0`. Строки MID: `pt` и `poly`. Утилиты: `bin\Win32\Release`.

| Команда | Код | Что прочитано обратно |
|---|---|---|
| `tab2tab sample.mif sample.tab`, затем `tab2tab sample.tab round.mif` | 0, 0 | `Point 10 20`, полигон `0 0 / 10 0 / 10 10 / 0 10 / 0 0` |
| `ogr2ogr -f "MapInfo File" via_ogr.tab sample.mif`, затем `ogrinfo via_ogr.tab via_ogr` | 0 | `POINT (9.989999999999998 19.994999999999997)`, полигон с `9.989999999999998` и `10.004999999999999` |

`ogrinfo` обоих TAB показывает `PROJECTION["Mollweide"]`, datum `WGS_1984`. Флага `-t_srs` не было, отдельного перехода в другую СК утилита не запрашивала.

В `sample.map` по смещению `0x170` лежит double `1000.0`. В `via_ogr.map` по `0x170` лежит `33.333333333333336`, по `0x178` — `66.66666666666667`. Это `m_XScale` и `m_YScale`: `TABMAPHeaderBlock` пишет их подряд (`mitab_mapheaderblock.cpp`, строки 859–860).

### 7.2. Почему масштабы разные

`tab2tab` после `SetSpatialRef` копирует границы источника, только если `GetBounds` вернул 0 (`mitab/tab2tab.cpp`, строки 248–250). У MIF без слова `Bounds` флаг `m_bBoundsSet` не ставится: разбор `CoordSys` читает границы лишь при четырёх числах после `Bounds` (`mitab_miffile.cpp`, строки 585–592). `PreParseFile` считает экстент объектов и `m_bBoundsSet` не включает. `GetBounds` тогда возвращает -1 (строки 2163–2170).

`SetSpatialRef` вызывает `SetProjInfo`, а тот берёт границы из `MITABLookupCoordSysBounds` (`mitab_tabfile.cpp`, строки 2406–2409). Запись проекции 13 в таблице (`mitab_bounds.cpp`, строка 979) — эллипсоид 7, единицы 7, сдвиги датума `-8, 160, 176`. Datum 104 в `mitab_spatialref.cpp` (строка 222) — `WGS_1984`, сдвиги `0, 0, 0`, эллипсоид 28. Совпадения нет, поиск границ возвращает ложь. `SetCoordsysBounds` не вызывается. Остаётся начальный масштаб `m_XScale = 1000.0` (`mitab_mapheaderblock.cpp`, строки 222–223). Обратный перевод `n / 1000` для 10 и 20 точный. В `round.mif` поэтому появились `Bounds (-1000000, -1000000) (1000000, 1000000)`: это диапазон целых `±1e9` при масштабе 1000.

`OGRTABDataSource::CreateLayer` (`mitab_ogr_datasource.cpp`, строки 412–421) после `SetSpatialRef` проверяет `IsBoundsSet`. Для корня `GEOGCS` он ставит `±1000`. Иначе, а Mollweide приходит как `PROJCS`, ставит:

```text
SetBounds(-30000000, -15000000, 30000000, 15000000)
```

`SetCoordsysBounds` считает масштаб как `2e9 / (max - min)` (строки 656–660) и пишет квадрант 1 (`HDR_DEF_ORG_QUADRANT` = 1, `mitab_mapheaderblock.cpp`, строка 148).

- X: ширина `60000000`, масштаб `2e9/6e7 = 100/3 ≈ 33.333333333333336`. `round(10 * 100/3) = 333`, обратно `333 * 3 / 100 = 9.99`.
- Y: высота `30000000`, масштаб `200/3 ≈ 66.66666666666667`. `round(20 * 200/3) = 1333`, обратно `1333 * 3 / 200 = 19.995`.
- Та же формула для Y=10: `round(10 * 200/3) = 667`, обратно `10.005`, на печати `10.004999999999999`.

Числа `ogrinfo` совпали с этой арифметикой. Дробные хвосты `9.989999999999998` и `19.994999999999997` — запись double для 9.99 и 19.995.

### 7.3. Вывод

Это свойство формата TAB: координата хранится целым числом в диапазоне примерно `±1e9`, а шаг равен размеру заданных границ. Mollweide здесь выбирает ветку «не GEOGCS» в `CreateLayer`. Отдельной ошибки преобразования проекции на этом файле не видно.

`Coordsys2Int` и `Int2Coordsys` не менять. Менять границы по умолчанию в `CreateLayer` тоже не стоит в этом этапе: от них зависит шаг всей таблицы, и для координат порядка десятков метров при ширине 60 000 000 м шаг около 0.03 по X и 0.015 по Y. Если позже понадобится, чтобы `ogr2ogr` сохранял 10 и 20 так же, как `tab2tab`, это отдельное решение: передавать в TAB явные Bounds, а не сужать формулу квантования. На этом аудите код границ не предлагается к правке.

Минимальный тест для повторной проверки — MIF из раздела 7.1 и две команды `tab2tab` и `ogr2ogr` выше. Ожидание: `tab2tab` возвращает `10 20`, `ogr2ogr` возвращает 9.99 и 19.995, в `.map` масштабы 1000 и `100/3` с `200/3`.

## 8. Delphi и C API

Сверка имён скриптом по `mitab/mitab_capi.h` и `contrib/mitab_dyn.pas`: в заголовке 93 функции `mitab_c_*`, в модуле 93 типа `Tmitab_c_*` и 93 вызова `GenericGetProcAddress`. Разницы множеств нет. Все 93 строки `_mitab_c_<name>@N`, собранные из пар `(имя, размер)` в `mitab_dyn.pas`, совпали с экспортами Win32 DLL. На x64 те же 93 имени есть без декорации.

`git diff upstream/master -- mitab/mitab_capi.h` и `git diff 73aef92 -- mitab/mitab_capi.h mitab/mitab.h` пустые. На этой ветке публичный заголовок C API и `mitab.h` не менялись. Незадокументированной смены сигнатур нет.

### 8.1. Соглашение вызова

Заголовок: `MITAB_STDCALL` = `__stdcall` на Win32 (`mitab_capi.h`, строки 172–175). x64 MSVC имя не декорирует; вызов прошёл через `CallingConvention.StdCall` в 64-битном тесте и вернул `2000000`.

`contrib/mitab_dyn.pas`, `GenericGetProcAddress` (строки 408–411) всегда запрашивает `'_'+имя+'@'+размер`. Запасного поиска без декорации нет. На Win32 это совпало с DLL. На x64 такого экспорта нет: 64-битный `GetProcAddress` для `_mitab_c_getlibversion@0` вернул пусто. Загрузчик дойдёт до `MITAB_LoadErr` «функция версии не найдена» и не свяжет остальные 92 имени.

Внешний `csMitabDyn.pas` (только чтение, в репозиторий не копировался) сначала ищет декорированное имя и при пустом результате повторяет поиск без `@N` (строки 547–557). Для x64 это как раз нужный порядок. Модуль в репозитории так не делает.

### 8.2. Типы

C API (`mitab_capi.h`, строки 186–188): `mitab_handle`, `mitab_feature`, `mitab_projinfo` — это `void *`. `int` у MSVC 32-битный и на Win32, и на x64.

`contrib/mitab_dyn.pas`, строки 122–124: все три типа — `Longword`. В Delphi `Longword` всегда 32 бита. На Win32 размер совпадает с указателем. На x64 указатель 64-битный, и `Longword` обрезает хэндл. Возврат `mitab_c_open` в таком модуле на x64 не сохранить.

Аргументы строк в `mitab_dyn.pas` объявлены как `pchar` (23 вхождения, `pAnsiChar` нет). В Delphi 2009 и новее `PChar` — это `PWideChar`. C API принимает `const char *`. Внешний `csMitabDyn.pas` использует `pAnsiChar` и `THandle`. Какой компилятор Delphi будет собирать репозиторный модуль, этот аудит не запускал. По тексту модуля x64-сборка и Unicode-сборка с C API не совпадают. Win32 и старый ANSI Delphi по размерам указателя и `char` совпадают с заголовком.

`longint` для полей `int` на обеих платформах MSVC совпадает по ширине: оба 32 бита. Размер стека `@N` в Win32 от этого не плывёт, и сверка с экспортами это подтвердила.

### 8.3. `mitab_c_set_affine_params`

C (`mitab_capi.h`, строки 420–422, и определение в `mitab_capi.cpp`, строка 2924): `void`, аргументы `mitab_projinfo`, `int`, `double *`. На Win32 стек 4+4+4 = 12. Экспорт `_mitab_c_set_affine_params@12` есть.

`contrib/mitab_dyn.pas`, строка 266: это `function ...: longint`, не `procedure`. Размер в `GenericGetProcAddress` равен 12 (строка 519), имя находится. Вызывающий код при этом читает EAX, а функция C EAX не задаёт. Стек stdcall это не ломает. Результат функции в Delphi не является кодом возврата C.

`contrib/mitab.pas`, строка 241, объявляет ту же функцию как `procedure` и внешнее имя `_mitab_c_set_affine_params@12`. Статическая привязка с заголовком совпадает, динамическая — нет.

`mitab_c_get_affine_params` в заголовке возвращает `int` (строки 417–419). В `mitab_dyn.pas` это `function ...: longint`. Это совпадает. Внешний `csMitabDyn.pas` объявляет обе affine-функции как `procedure` (строки 282–283): `set` совпадает с `void`, `get` теряет код «0, если affine нет».

Проверка версии в `mitab_dyn.pas` (строки 136–137 и 440) принимает `Libversion` `1007000` или `LibversionOr` `2000000`. Текущая DLL возвращает `2000000`, порог для неё проходит. Константу `2000000` не менять.

Сборки Delphi не было. Совпадение имён и размеров стека — это сверка текстов и экспортов, не интеграционный прогон.

## 9. Что менять, когда будет подтверждение

Порядок такой. Каждый пункт отдельно, со своей проверкой. C API, `__stdcall` и `MITAB_VERSION_INT` не входят ни в один пункт.

1. Добавить корневой `LICENSE` (текст из `README.TXT`) и `NOTICE` (дословно `ogr/swq.c` и `cpl/cpl_recode_stub.cpp`, восемь SHA GDAL, copyright Even Rouault). В трёх файлах `mitab_feature.cpp`, `mitab_mapindexblock.cpp`, `mitab_mapobjectblock.cpp` добавить строку copyright 2014 Even Rouault. В `README.md` заменить фразу про одну лицензию на отсылку к `LICENSE` и `NOTICE`. Поведение кода не меняется.
2. Поправить документы, которые расходятся с деревом: абзац про неполный `mitab_dyn.pas` в `docs/DEVELOPMENT_REPORT.md` (сейчас там все 93 загрузки), комментарий «exports only the stdcall C API» в `MITAB.Utility.props` оставить на момент после сокращения экспортов. Убрать из отчёта абсолютный путь `C:\Users\grig\...` или заменить его на имя ветки и remote.
3. В `.gitignore` добавить `mitab.code-workspace` и `*.exe`. Файл workspace в коммит не класть.
4. Включить `CPL_DISABLE_DLL` в CPL, OGR и MITAB. Не добавлять `.def`. Пересобрать `Release|Win32` и `Release|x64`. Сверить 93 экспорта.
5. Переименовать `TargetName` / `OUTPUT_NAME`, как в разделе 5. Обновить строки имени файла в документации и привязках. Экспорты не трогать. Повторить `LoadLibrary`.
6. Для x64 в vcxproj поставить subsystem `5.02`, как уже делает `CMakeLists.txt`. Win32 оставить `5.01`. Это меняет только формальный номер PE.
7. В `contrib/mitab_dyn.pas`, не в `csMitabDyn.pas`: `mitab_c_set_affine_params` сделать `procedure`; для x64 искать имя без `@N`, если декорированное не найдено; тип хэндла и `PChar` согласовать с `void*` и `const char*` так, чтобы Win32 stdcall и размеры `@N` не изменились. Это правка привязки, не DLL. После неё всё равно нужна сборка Delphi, которой в этом аудите не было.
8. Координатный код не менять. В документацию можно перенести описание из раздела 7, чтобы следующее чтение `9.99 19.995` не выглядело как регрессия Mollweide.

`ogr_srs_esri_names.h` и лицензия `contrib/dotnet` остаются открытыми, пока не найден исходный текст разрешения. Их не закрывать выдуманным заголовком.

## 10. Проверка после этих правок

1. `git diff -- mitab/mitab_capi.h mitab/mitab.h` пустой. `MITAB_VERSION_INT` равен `2000000`.
2. `msbuild MITAB.sln /t:Rebuild /p:Configuration=Release /p:Platform=Win32` и то же для x64, MSBuild 12.0, toolset `v120_xp`. Код выхода 0.
3. `dumpbin /exports` Win32: ровно 93 имени, все `_mitab_c_*@N`, набор совпадает с текущим списком. x64: ровно 93 имени без `@`. `dumpbin /dependents`: по-прежнему только `KERNEL32.dll`.
4. `dumpbin /headers`: Win32 subsystem 5.01, x64 subsystem 5.02, если пункт 6 сделан. Иначе x64 останется 6.00, и это нужно так и записать.
5. Повтор `LoadLibrary` / `GetProcAddress` из раздела 5 на новых именах файлов. Win32 вызывает `_mitab_c_getlibversion@0`, x64 вызывает `mitab_c_getlibversion`, оба дают `2000000`.
6. Повторить MIF из раздела 7. Ожидание то же: `tab2tab` сохраняет `10 20`, `ogr2ogr` даёт 9.99 и 19.995. Если числа изменились, правка задела границы или квантование, хотя не должна была.
7. `tabdump` без аргументов печатает строку про `-DDEBUG` и возвращает -1. Макроса `DEBUG` в `tabdump.vcxproj` нет, `main` без него — строки 918–921 `mitab/tabdump.cpp`.
8. `git status`: нет `*.dll`, `*.exe`, `*.obj`, `*.lib`, нет `mitab.code-workspace` в индексе. `git grep` по абсолютным путям пустой.
9. Рядом с собранными DLL лежат `LICENSE` и `NOTICE`. В `NOTICE` есть абзацы из `swq.c` и `cpl_recode_stub.cpp`.
10. Delphi: собрать внешний клиент отдельно, уже после того как путь к новой DLL задан в его константах. Пока этой сборки нет, совместимость Delphi не считать подтверждённой. Запуск на Windows XP — отдельный пункт на самой XP, его результат сюда не переносить заранее.
