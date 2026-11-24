# Отчёт о текущем этапе MITAB

Дата проверки: 11 ноября 2026. Хост: Windows 10.0.19045. Репозиторий: `C:\Users\grig\MITAB-Analysis\repositories\mapgears`, ветка `fix/windows-xp-build`. Remote `origin` — `https://github.com/grig27/mitab.git`, `upstream` — `https://github.com/mapgears/mitab.git`.

Этот файл описывает уже сделанную работу и проверки, которые реально выполнялись. Push не выполнялся.

## 1. Исходное состояние и точка расхождения с GDAL

Содержательная точка расхождения зафиксирована в `docs/gdal-migration/ANALYSIS.md` и не пересматривалась:

| Сторона | SHA | Дата | Смысл |
|---|---|---|---|
| MITAB | `73aef92ed8e311075b46a4d5b384218a59715951` | 2011-09-22 | Последний содержательный коммит библиотеки до паузы: tab-разделитель в MIF, GDAL ticket 4257 |
| GDAL | `b075f49fca` | 2011-09-23 | Resync with upstream MITAB по тому же ticket |

История GDAL после этого коммита в драйвер MapInfo не переносилась целиком. В наш `mitab/` попали только пункты, отмеченные ниже как реализованные.

До финализации в локальной истории уже были четыре коммита этапа: сборка Visual Studio (`f5a9108`), журнал миграции (`4af56c2`) и два коммита исправлений (`5caa262`, `72c7898`). Ветка совпадала с `origin/fix/windows-xp-build` на `72c7898`. Незакоммиченными оставались проекты утилит, правки `MITAB.sln`, привязки в `contrib/` и `mitab.code-workspace`.

## 2. Система сборки

Коммит `f5a9108c42a3a4c462dc914da167faab037606ff` добавил:

- `MITAB.sln`, `CPL.vcxproj`, `OGR.vcxproj`, `MITAB.vcxproj`, `MITAB.Common.props`
- `CMakeLists.txt`
- `cpl/cpl_config.h` и правку `cpl/cpl_port.h`
- `.gitignore` и `.gitattributes`

Схема каталогов: `bin\<Platform>\<Configuration>\` и `obj\<Project>\<Platform>\<Configuration>\`. CPL и OGR — статические библиотеки. MITAB — `mitab.dll`. Toolset во всех конфигурациях проектов — `v120_xp`. Набор конфигураций решения: `Debug|Win32`, `Debug|x64`, `Release|Win32`, `Release|x64`. CRT статический: `/MT` в Release, `/MTd` в Debug. Командная строка компилятора пишет `VC120.PDB` и `_USING_V110_SDK71_`.

`CMakeLists.txt` собирает те же три библиотеки и задаёт subsystem 5.01 для Win32 и 5.02 для 64-bit. Утилиты CMake не собирает. CMake в этот день не запускался.

Обязательная сборка этого этапа — `Release|Win32`, MSBuild 12.0, toolset `v120_xp`.

## 3. Консольные утилиты

В решение добавлены `tab2tab`, `tabdump`, `ogrinfo` и `ogr2ogr`. Общие настройки линковки — `MITAB.Utility.props`.

Утилиты вызывают классы C++ MITAB. `mitab.dll` экспортирует stdcall C API, поэтому утилиты не линкуют её импортную библиотеку. Перед линковкой в командную строку попадают объектные файлы из `obj\MITAB\<Platform>\<Configuration>\*.obj`, плюс статические `cpl.lib` и `ogr.lib`. Ссылка на проект MITAB стоит с `LinkLibraryDependencies=false`. Для `tab2tab` Win32 это подтверждено `link.command.1.tlog`: набор `.obj` MITAB, обе статические библиотеки, `/MACHINE:X86`, `/SUBSYSTEM:CONSOLE,5.01`. `ogrinfo` и `ogr2ogr` дополнительно компилируют `mitab_ogr_driver.cpp` и `mitab_ogr_datasource.cpp`. Отдельная статическая библиотека MITAB не вводилась.

`tabdump` в этой конфигурации собирается без макроса `DEBUG`. Рабочий разбор блоков закрыт `#ifdef DEBUG` в `mitab/tabdump.cpp`. Собранный `tabdump.exe` печатает `tabdump is only available when MITAB is compiled with -DDEBUG option.` и возвращает -1. Глобальный `DEBUG` для DLL не включался: в `mitab.h` и `mitab_priv.h` под ним объявлены виртуальные `Dump`, то есть меняется раскладка vtable.

## 4. Исправления MITAB

Журнал: `docs/gdal-migration/PROGRESS.md`. Сверка с кодом на 11 ноября 2026:

| ID | Исправление | GDAL SHA | Коммит MITAB | Статус |
|---|---|---|---|---|
| 1 | `break` после Mollweide, проекция 13 не падает в Eckert IV | `a5cbb6c535` | `5caa26224213b232f0a165e015b4ae034a1a6380` | VERIFIED |
| 3 | Нормализация целого MBR в `TABFeature::UpdateMBR` | `2789405d5b` | `5caa26224213b232f0a165e015b4ae034a1a6380` | VERIFIED |
| 4 | `closeRings()` при чтении Region из MIF | `1a9fbce231` | `5caa26224213b232f0a165e015b4ae034a1a6380` | VERIFIED |
| 6 | Радиус ROUNDRECT: один токен и индекс `[0]` | `62d8f0bb3d` | `5caa26224213b232f0a165e015b4ae034a1a6380` | VERIFIED |
| 5 | Пустой Region не вызывает `getEnvelope` | `bc34cd533c` | `72c78981d5a5e5c95a28412d21878cfc889329f4` | VERIFIED |
| 7 | Отказ на обрезанном PLINE / PLINE MULTIPLE | `588cc86485` | `72c78981d5a5e5c95a28412d21878cfc889329f4` | VERIFIED |
| 8 | `PickSeedsForSplit` при нулевой ширине или высоте | `2c5c3db03f` | `72c78981d5a5e5c95a28412d21878cfc889329f4` | VERIFIED |
| 9 | Отрицательное `numLineSections` до `CPLMalloc` | `6edba1e791` | `72c78981d5a5e5c95a28412d21878cfc889329f4` | VERIFIED |

В коде это видно так: `case 13` в `mitab_coordsys.cpp` и `mitab_spatialref.cpp` вызывает `SetMollweide` и делает `break`; `UpdateMBR` меняет местами целые min/max; `ReadGeometryFromMIFFile` для Region вызывает `closeRings()` и берёт конверт только если геометрия создана; `PickSeedsForSplit` подставляет 0 при нулевом протяжении; `ReadObj` возвращает -1 при `m_numLineSections < 0`.

Пункты 2 и 10 в коде не перенесены. `m_XPrecision` / `UpdatePrecision` нет. `MIFFile::Open` по-прежнему возвращает -1, если сразу после заголовка `GetLine()` у MID даёт `NULL` (`mitab_miffile.cpp`, около строки 422). Статус обоих пунктов оставлен `TODO`.

Поведение пунктов 3, 6, 8 и 9 в этот день заново не прогонялось. Их статус `VERIFIED` держится на проверках, уже записанных в строках журнала, и на том, что соответствующий код входит в названные коммиты. Новые пункты миграции не переносились. Пункт 23 остаётся `TODO` с пометкой «не переносить как есть».

## 5. Публичный C API

Сравнение `mitab/mitab_capi.h` и `dumpbin /exports` после Rebuild.

В заголовке 93 функции `mitab_c_*`. Размеры стека для `__stdcall` посчитаны по объявлениям: `double` по значению — 8 байт, указатели и `int` — 4 байта на Win32.

| Платформа | Образ | Экспортов всего | `mitab_c_*` | Совпадение с заголовком |
|---|---|---:|---:|---|
| Win32 | PE32, machine `14C`, subsystem 5.01, Windows GUI | 1770 | 93 | все 93 имени вида `_mitab_c_<name>@<bytes>`, лишних `mitab_c_*` нет |
| x64 | PE32+, machine `8664`, subsystem 6.00, Windows GUI | 1770 | 93 | все 93 имени без декорации, лишних `mitab_c_*` нет |

Сигнатуры и набор `mitab_c_*` не менялись. Большое общее число экспортов даёт `CPL_DLL` в `cpl/cpl_port.h`: на MSVC он по умолчанию `__declspec(dllexport)`, поэтому в DLL попадают и символы CPL/OGR. Это наблюдаемое состояние текущей сборки, его не перекраивали.

Внешний клиент на Delphi в этот репозиторий не копировался и не изменялся. Повторной сборки этого клиента в ходе финализации не было. Проверка совместимости здесь — совпадение 93 экспортов с текущим заголовком и с ранее зафиксированным контрактом stdcall.

Привязки в `contrib/` обновлены под уже существующий заголовок: добавлены объявления функций, которые DLL и так экспортирует, у `mitab_c_get_field_as_double` алиас указывает на `_mitab_c_get_field_as_double@8`. `contrib/mitab_dyn.pas` по-прежнему загружает не весь набор: в нём нет функций коллекции, проекции, датума и affine. Эти файлы компилятором Delphi, VB или C# в этот день не собирались.

## 6. Сборки

Обе команды — MSBuild 12.0, `MITAB.sln`, `/t:Rebuild`, toolset из проектов `v120_xp`.

### Release|Win32

Команда завершилась с кодом 0. Собраны `cpl.lib`, `ogr.lib`, `mitab.dll`, `tab2tab.exe`, `tabdump.exe`, `ogrinfo.exe`, `ogr2ogr.exe` в `bin\Win32\Release\`.

Предупреждения, которые напечатал этот Rebuild:

- `cpl/cpl_vsi_mem.cpp(212)`: C4244, `vsi_l_offset` в `size_t`
- `ogrsurface.obj`: LNK4221, объект не определяет ранее не определённые символы
- `mitab/mitab_feature_mif.cpp(312)`: C4065, `switch` с `default` и без `case`

Они были и до финализации. Образ `mitab.dll`: subsystem 5.01. Сама Windows XP не загружалась; прогон шёл на Windows 10.

### Release|x64

Toolset `v120_xp` для x64 установлен, кросс-компилятор `VC\bin\x86_amd64\cl.exe` есть. Rebuild завершился с кодом 0. Те же пять выходных файлов лежат в `bin\x64\Release\`.

Помимо C4065 и LNK4221 того же рода, компилятор печатал C4267 и C4244 в `cpl/`, `ogr/` и `mitab/` (сужение `size_t` / `__int64` к `int`). Точный подсчёт строк предупреждений не снимался: журнал x64 в консоли был обрезан. Линкер напечатал LNK4010 для `mitab.dll` и четырёх утилит: номер subsystem 5.01 для x64 недопустим, принята версия по умолчанию. `dumpbin /headers` показывает subsystem 6.00. Это не образ Windows XP и не проверка на Windows XP x64. Утилиты x64 запускались на этом же Windows 10.

`Debug|Win32` и `Debug|x64` не собирались.

## 7. Тесты

Готового набора autotest GDAL в дереве нет. Прогон — временные MIF/MID в `%TEMP%\mitab-final-check2`, ASCII без BOM. Первый заход с пустыми MID ожидаемо не открыл файлы; ниже результаты повторного прогона, где у каждого объекта есть строка MID. Код выхода -1 утилиты в отчёте Python виден как `4294967295`.

Одинаково для `bin\Win32\Release` и `bin\x64\Release`:

| Проверка | Результат |
|---|---|
| `tab2tab` без аргументов | exit 1, текст версии `2.0.0-dev (2008-10)` и справка |
| `tab2tab sample.mif sample.tab` | exit 0 |
| `tab2tab sample.tab round.mif` | exit 0. В MIF строка `CoordSys Earth Projection 13, 104, "m", 0`. Незамкнутый Region из 4 точек записан как `Region 1` из 5 точек, первая и последняя `0 0` |
| `ogrinfo sample.mif` и `ogrinfo sample.tab` | exit 0, драйвер `MapInfo File`, один слой |
| `ogr2ogr -f "MapInfo File" via_ogr.tab sample.mif` | exit 0. `ogrinfo via_ogr.tab` открывает результат |
| `ogrinfo eckert.mif` | exit 0 |
| `ogrinfo emptyreg.mif` (`Region 0` и строка MID) | exit 0, процесс не падает |
| `tab2tab badpline.mif` (PLINE на 3 вершины, в файле одна координата) | exit -1, `Failed to read feature 1.` и `ERROR 6: Error during reading Geometry.` |
| `ogrinfo nomid.mif` (заголовок MIF и пустой MID) | exit 1, `Unable to open datasource` |
| `tabdump` без аргументов и `tabdump -a sample.tab` | exit -1, сообщение про `-DDEBUG` |

Подробный `ogrinfo <файл> <слой>` на Win32:

- `sample.mif` и TAB после `tab2tab`: Feature Count 3, `PROJECTION["Mollweide"]`, полигон `(0 0,10 0,10 10,0 10,0 0)`.
- `eckert.mif`, `Earth Projection 14`: Feature Count 1, `PROJECTION["Eckert_IV"]`.
- `emptyreg.mif`: Feature Count 1, экстент `(0, 0) - (0, 0)`, у объекта есть атрибут и стиль, строки геометрии полигона нет.
- `via_ogr.tab` после `ogr2ogr`: Feature Count 3, снова Mollweide. Координаты целого TAB при чтении сместились: точка `10 20` прочитана как `POINT (9.989999999999998 19.994999999999997)`. Это наблюдение данного прогона, отдельным исправлением не закрывалось. Путь `tab2tab` для того же MIF вернул в `round.mif` исходные `10 20`, `10 10`, `20 20`, `30 10`.

Так этот прогон заново показывает пункты 1, 4, 5 и 7. Пункт 10 подтверждён как ещё не исправленный: пустой MID не открывается. Пункты 3, 6, 8 и 9 этим набором файлов не покрыты.

## 8. Известные ограничения

- `tabdump` в `Release|Win32` и `Release|x64` не разбирает файлы, пока проект MITAB не собран с `DEBUG`. Такой сборки нет, потому что она меняет C++ ABI DLL.
- x64-образ получает subsystem 6.00 из-за LNK4010. В проектах для x64 по-прежнему записан 5.01, как и для Win32.
- Windows XP как ОС не проверялась. Проверялись образ Win32 с пометкой 5.01 и запуск на Windows 10.
- Пустой MID после заголовка MIF не открывается. Это текущее поведение пункта 10, не целевое.
- Округление координат в `Int2Coordsys` (пункт 2) не сделано.
- `ogr2ogr` в TAB на пробном файле дал целые координаты с видимым шагом около 0.01 при чтении обратно. `tab2tab` на том же MIF сохранил записанные значения.
- `contrib/mitab_dyn.pas` покрывает не все 93 функции.
- CMake и конфигурации Debug не запускались.
- Предупреждения C4244, C4267, C4065 и LNK4221 оставлены: они не из сегодняшних правок журнала и утилит.

## 9. Нереализованные пункты миграции

`TODO`: 2, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23.

`BLOCKED`: B1 LargeInt / `OFTInteger64`, B2 UTF-8 через `CPLRecode`, B3 датум из WKT2, B4 `AlterFieldDefn`, B5 новые коды проекции 34 и 35, если для них нужен новый `SRS_PT_*`.

`SKIPPED`: B6 (`TestCapability` как `bool`, новый `ICreateLayer`, плагин драйвера).

Пункт 22 зависит от отсутствующего `TABFile::SetFeature`. Пункт 23 не копировать как есть: diff GDAL прячет ошибку преобразования коллекции. Рядом в `ANALYSIS.md` перечислены дополнительные SHA, которых нет в нумерованных 23 пунктах; они в работу не брались.

## 10. Следующие этапы

1. Пункты 2 и 10: это оставшаяся первая группа, оба ещё `TODO`. Для 10 достаточно внутреннего флага, без правки сигнатуры в `mitab.h` и без C API.
2. Пункты 11–21 по одному, с проверкой из `ANALYSIS.md` и сборкой `Release|Win32` `v120_xp`.
3. Пункт 22 только вместе с реализацией записи объекта по идентификатору.
4. Пункт 23 — отдельное решение, код до него не трогать.
5. B1–B5 не начинать без разрешения менять `cpl/` или `ogr/`.
6. Для x64 имеет смысл отдельно решить, ставить ли subsystem 5.02, как уже делает `CMakeLists.txt`. Сейчас проекты этого не делают, и линкер подставляет 6.00.
7. Полноценный `tabdump` требует отдельного решения по `DEBUG`, без включения этого макроса в сборку `mitab.dll`.

## 11. Коммиты

Запрошенные группы «документация миграции» и «исправления GDAL» уже были в истории, повторно не создавались. Привязки `contrib/` не входили ни в утилиты, ни в документацию, поэтому для них сделан отдельный коммит. `mitab.code-workspace` не включён: в нём абсолютные пути к GDAL и к внешнему дереву Delphi.

| SHA | Когда | Содержание |
|---|---|---|
| `f5a9108c42a3a4c462dc914da167faab037606ff` | уже был | Сборка Visual Studio: `CPL.vcxproj`, `OGR.vcxproj`, `MITAB.vcxproj`, `MITAB.Common.props`, `MITAB.sln`, `CMakeLists.txt`, `cpl/cpl_config.h`, `cpl/cpl_port.h`, `.gitignore`, `.gitattributes` |
| `4af56c26b9b2af5d60ca5894878ca3c9e07ea114` | уже был | `docs/gdal-migration/ANALYSIS.md`, `docs/gdal-migration/PROGRESS.md`, `.cursor/rules/mitab-migration.mdc` |
| `5caa26224213b232f0a165e015b4ae034a1a6380` | уже был | Пункты 1, 3, 4, 6: `mitab_coordsys.cpp`, `mitab_spatialref.cpp`, `mitab_feature.cpp`, `mitab_feature_mif.cpp`, строка журнала |
| `72c78981d5a5e5c95a28412d21878cfc889329f4` | уже был | Пункты 5, 7, 8, 9: `mitab_feature_mif.cpp`, `mitab_mapindexblock.cpp`, `mitab_mapobjectblock.cpp`, строка журнала |
| `4fcc06e0f4a9a271adf2978340dfe1f8ac7b8ac9` | финализация | Утилиты: `MITAB.sln`, `MITAB.Utility.props`, `tab2tab.vcxproj`, `tabdump.vcxproj`, `ogrinfo.vcxproj`, `ogr2ogr.vcxproj` |
| `5defcf8b65cc1c47e89aa278d5d86c60216edcaf` | финализация | Привязки: `contrib/mitab.def`, `contrib/mitab.pas`, `contrib/mitab.vb`, `contrib/mitab_cdecl.pas`, `contrib/mitab_dyn.pas`, `contrib/dotnet/MiApi.cs`, `contrib/dotnet/README_DOTNET.TXT` |
| `bd6333627c1b0c348cdc4ee1c0e5e46002a6be89` | финализация | `README.md`, `CONTRIBUTING.md`, актуальные SHA в `PROGRESS.md`, правка правила сборки в `.cursor/rules/mitab-migration.mdc` |

Этот отчёт добавляется следующим коммитом. Его SHA в этот текст не вписан: коммит создаётся уже после файла.
