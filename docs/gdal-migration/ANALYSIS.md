# Анализ расхождения MITAB и GDAL после сентября 2011

Документ фиксирует сравнение самостоятельной библиотеки MITAB и драйвера MapInfo в GDAL. Повторный просмотр всех 521 коммита GDAL не нужен: выводы, SHA и места в коде собраны здесь.

Источники на момент анализа (11 ноября 2026):

| Дерево | Путь | Remote |
|---|---|---|
| Наш форк | `C:\Users\grig\MITAB-Analysis\repositories\mapgears` | `https://github.com/grig27/mitab` (`origin`), upstream `https://github.com/mapgears/mitab` |
| GDAL, только чтение | `C:\Users\grig\MITAB-Analysis\repositories\gdal` | `https://github.com/OSGeo/gdal` |

Драйвер GDAL: `ogr\ogrsf_frmts\mitab`. Код систем координат после 2024 года лежит отдельно, в `ogr\ogrmitabspatialref.cpp`.

Ветка нашего репозитория на момент анализа: `fix/windows-xp-build`. Коммит `f5a9108` («Add Visual Studio build support for MITAB») меняет только сборку Visual Studio. Исходники `mitab/` относительно upstream он не меняет.

## Точка расхождения

| Сторона | SHA | Дата | Смысл |
|---|---|---|---|
| MITAB | `73aef92ed8e311075b46a4d5b384218a59715951` (`73aef92`) | 2011-09-22 | Последний содержательный коммит библиотеки: tab-разделитель в MIF, GDAL ticket 4257 |
| GDAL | `b075f49fca` | 2011-09-23 | «Resync with upstream MITAB CVS» по тому же ticket 4257 |

`73aef92` не является предком текущего `master` GDAL под тем же хешем: история GDAL переписывалась при переносе из SVN. Содержательно это одна и та же синхронизация. Всё, что в GDAL попало в драйвер после `b075f49fca`, в нашем дереве отсутствует, если ниже явно не сказано обратное.

На `master` GDAL после 2011-09-24 путь драйвера задевают **521 коммит**. До смены корня репозитория путь был `gdal/ogr/ogrsf_frmts/mitab`. Журнал только по новому пути `ogr/ogrsf_frmts/mitab` обрывается на 2007 и снова появляется в 2021: коммиты 2008–2020 есть, но лежат на старом префиксе. Искать их нужно так:

```
git log master --since="2011-09-24" -- "gdal/ogr/ogrsf_frmts/mitab" "ogr/ogrsf_frmts/mitab"
```

Большая часть 521 коммита — стиль, `clang-tidy`, `final`/`override`, смена `TestCapability()` с `int` на `bool`, правки сборки. Ниже только то, что меняет поведение TAB/MIF.

Код проекций уехал из каталога драйвера коммитом `b7830458db` (2024-09-22). Для переноса CRS источником служит история `mitab_spatialref.cpp` / `mitab_coordsys.cpp` и текущий `ogr\ogrmitabspatialref.cpp`. Место правки у нас — по-прежнему `mitab/mitab_spatialref.cpp` и `mitab/mitab_coordsys.cpp`.

## Сопоставление файлов

Общие исходники (строки нашего файла → строки GDAL, разница):

| Файл | Наш | GDAL | Разница |
|---|---:|---:|---:|
| `mitab.h` | 1606 | 1783 | +177 |
| `mitab_datfile.cpp` | 1669 | 2404 | +735 |
| `mitab_feature.cpp` | 7870 | 8297 | +427 |
| `mitab_feature_mif.cpp` | 2203 | 2052 | −151 |
| `mitab_geometry.cpp` | 380 | 352 | −28 |
| `mitab_geometry.h` | 49 | 23 | −26 |
| `mitab_idfile.cpp` | 347 | 327 | −20 |
| `mitab_imapinfofile.cpp` | 479 | 669 | +190 |
| `mitab_indfile.cpp` | 1892 | 1912 | +20 |
| `mitab_mapcoordblock.cpp` | 799 | 782 | −17 |
| `mitab_mapfile.cpp` | 2513 | 2827 | +314 |
| `mitab_mapheaderblock.cpp` | 955 | 885 | −70 |
| `mitab_mapindexblock.cpp` | 1359 | 1282 | −77 |
| `mitab_mapobjectblock.cpp` | 1720 | 1814 | +94 |
| `mitab_maptoolblock.cpp` | 388 | 392 | +4 |
| `mitab_middatafile.cpp` | 287 | 262 | −25 |
| `mitab_miffile.cpp` | 1996 | 1896 | −100 |
| `mitab_ogr_datasource.cpp` | 381 | 489 | +108 |
| `mitab_ogr_driver.cpp` | 198 | 234 | +36 |
| `mitab_ogr_driver.h` | 130 | 59 | −71 |
| `mitab_priv.h` | 1514 | 1641 | +127 |
| `mitab_rawbinblock.cpp` | 1070 | 1135 | +65 |
| `mitab_tabfile.cpp` | 2228 | 2820 | +592 |
| `mitab_tabseamless.cpp` | 802 | 720 | −82 |
| `mitab_tabview.cpp` | 1817 | 1740 | −77 |
| `mitab_tooldef.cpp` | 657 | 634 | −23 |
| `mitab_utils.cpp` | 667 | 495 | −172 |
| `mitab_utils.h` | 86 | 36 | −50 |

Сильнее всего выросли `mitab_datfile.cpp`, `mitab_tabfile.cpp`, `mitab_feature.cpp`, `mitab_mapfile.cpp`. Это смесь проверок битых файлов, LargeInt, UTF-8 и адаптации к новому OGR, а не один алгоритм.

Только у нас, внутри `mitab/`:

- C API: `mitab_capi.cpp`, `mitab_capi.h`, `mitabc_test.c`
- CRS, которых в каталоге драйвера GDAL уже нет: `mitab_coordsys.cpp`, `mitab_spatialref.cpp`, `mitab_bounds.cpp`, `allproj.cpp`
- утилиты: `tab2tab.cpp`, `ogr2ogr.cpp`, `ogrinfo.cpp`, `tabdump.cpp`, `tabindex.cpp`
- сборка каталога: `makefile.vc`, `GNUmakefile`, `Doxyfile`, `MapInfoBounds.db`

Только в каталоге драйвера GDAL: `CMakeLists.txt`, `HISTORY.TXT`, `README.TXT`, `TODO.TXT`. У нас `HISTORY.TXT` и `README.TXT` лежат в корне репозитория. `HISTORY.TXT` внутри драйвера GDAL застыл около 2007–2008 и новее нашего корневого `HISTORY.TXT`: живая история GDAL после 2011 года — в git, не в этом файле.

## Сопоставление классов

Набор классов один и тот же. Новых классов геометрии GDAL не добавил. В GDAL у классов появились `final` / `CPL_NON_FINAL` и дополнительные методы. Это оформление C++ API GDAL, его не переносим.

| Класс | Роль |
|---|---|
| `IMapInfoFile` | Базовый слой OGR |
| `TABFile` | Чтение и запись `.tab` / `.map` / `.dat` / `.id` / `.ind` |
| `TABView` | Представление над несколькими TAB |
| `TABSeamless` | Бесшовный набор TAB |
| `MIFFile` | Чтение и запись MIF/MID |
| `TABFeature` | Базовая фича MapInfo |
| `TABPoint`, `TABFontPoint`, `TABCustomPoint` | Точки |
| `TABPolyline` | Полилиния, в том числе MULTIPLE |
| `TABRegion` | Полигон / регион |
| `TABRectangle`, `TABEllipse`, `TABArc` | Прямоугольник, эллипс, дуга |
| `TABText` | Подпись |
| `TABMultiPoint` | Мультиточка |
| `TABCollection` | Коллекция регион + полилиния + мультиточка |
| `TABDebugFeature` | Отладочный тип |
| `ITABFeaturePen`, `ITABFeatureBrush`, `ITABFeatureFont`, `ITABFeatureSymbol` | Стиль |

Блоки `.map`: `TABMAPFile`, `TABMAPHeaderBlock`, `TABMAPIndexBlock`, `TABMAPObjectBlock`, `TABMAPCoordBlock`, `TABMAPToolBlock`, заголовки объектов (`TABMAPObjPLine`, `TABMAPObjCollection` и соседние). Атрибуты: `TABDATFile`, `MIDDATAFile`, `TABINDFile`, `TABIDFile`.

## Что уже есть у нас и что к нам не относится

Эти коммиты GDAL не входят в список из 23 пунктов. Повторять их не нужно.

- Запись MIF уже пишет `Integer` и `SmallInt` без ширины. `MIFFile`, запись колонок, `mitab_miffile.cpp` около строк 1069–1076. Патч `b94f8ed7c9` (ticket 3853, «не писать `Integer(width)`») у нас уже в этом виде. В `AddFieldNative` ширина 0 по-прежнему подменяется на 254 до `switch`, но для `TABFInteger` / `TABFSmallInt` `SetWidth` не вызывается, так что в определение поля OGR это не попадает.
- Пустое поле Time при `MITAB_USE_OFTDATETIME` уже пишется как `-1,-1,-1`. `TABFeature::WriteRecordToDATFile`, ветка `TABFTime`, `mitab_feature.cpp` около строк 630–642. Патч `05d0088ab2` (GitHub #2612) уже покрыт.
- `TABFile::SetFeature(OGRFeature*)` возвращает `OGRERR_UNSUPPORTED_OPERATION`. `mitab_tabfile.cpp` около строк 1565–1578, внутри комментарий TODO. `DeleteFeature()` в нашем дереве нет (маркер удаления `0x40000000` тоже не найден). Патчи произвольной правки и удаления объектов чинят код, которого у нас нет. Это пункт 22: его имеет смысл брать только вместе с реализацией записи по идентификатору.
- Переполнение `EscapeString` (`96d6b7415c`, 2026-05-11) сидит в статической функции `EscapeString` в `mitab_tabfile.cpp` GDAL. Она ограничивает описание `MAX_DESCRIPTION_LEN` и удваивает кавычки. В нашем `mitab/` этой функции нет. Наш `TABEscapeString` в `mitab_utils.cpp` экранирует `\n` и `\\` и буфер `2 * strlen + 1` под этот алгоритм выделяет достаточно.

Номера строк в примерах ниже сняты с дерева на момент анализа, коммит `f5a9108` на ветке `fix/windows-xp-build`, до любого переноса. Если рабочее дерево уже содержит черновик правки, сверяться с `git show HEAD:mitab/<файл>`, а не с несохранённым буфером.

## Доказательства по нашему коду

Проекция 13 (Mollweide) проваливается в case 14 (Eckert IV): после `SetMollweide` нет `break`. То же место в `mitab_spatialref.cpp` около строк 791–800. Коммит GDAL `a5cbb6c535` добавляет ровно один `break` в каждом файле. В текущем GDAL `break` есть (`ogr\ogrmitabspatialref.cpp`, case 13). Case 13 в `mitab_feature.cpp` около строки 8283 — это стиль пера, не проекция. Его не трогать.

```528:538:mitab/mitab_coordsys.cpp
      case 13:
        poSR->SetMollweide( GetMIFParm( papszNextField, 0, 0.0 ),
                            0.0, 0.0 );

        /*--------------------------------------------------------------
         * Eckert IV
         *-------------------------------------------------------------*/
      case 14:
        poSR->SetEckertIV( GetMIFParm( papszNextField, 0, 0.0 ),
                           0.0, 0.0 );
        break;
```

`TABMAPHeaderBlock::Int2Coordsys` отдаёт частное без округления до точности масштаба. В GDAL после деления стоит `round(dX * m_XPrecision) / m_XPrecision`, а `UpdatePrecision()` считает точность как `pow(10.0, round(log10(m_XScale)))` и вызывается из `InitBlockFromData` и `SetCoordsysBounds`. Коммит `2c00f6704a`, [mapgears/mitab#2](https://github.com/mapgears/mitab/issues/2). Поля `m_XPrecision` / `m_YPrecision` у нас в `mitab_priv.h` отсутствуют. `round` из того коммита на Win32 был локальной функцией; для VS2013 допустим `std::round` из `<cmath>` (C++11).

```435:446:mitab/mitab_mapheaderblock.cpp
    if (m_nCoordOriginQuadrant==2 || m_nCoordOriginQuadrant==3 ||
        m_nCoordOriginQuadrant==0 )
        dX = -1.0 * (nX + m_XDispl) / m_XScale;
    else
        dX = (nX - m_XDispl) / m_XScale;

    if (m_nCoordOriginQuadrant==3 || m_nCoordOriginQuadrant==4||
        m_nCoordOriginQuadrant==0)
        dY = -1.0 * (nY + m_YDispl) / m_YScale;
    else
        dY = (nY - m_YDispl) / m_YScale;
```

Для квадрантов, которые меняют знак, `Coordsys2Int` превращает мировую нижнюю границу в большее целое, а верхнюю — в меньшее. `TABFeature::UpdateMBR` записывает результат как есть (`mitab_feature.cpp` около строк 743–744). GDAL после двух вызовов меняет границы местами, если `m_nXMin > m_nXMax` или `m_nYMin > m_nYMax`. Коммит `2789405d5b`, GitHub #817. Это путь создания объекта через `UpdateMBR` / `ValidateCoordType`, он у нас живой. `SetFeature` для этого патча не нужен. В коммите GDAL опечатка в комментарии: «Coordsy2Int». При переносе комментарий можно писать правильно, алгоритм — `std::swap` двух пар `GInt32`.

Region из MIF кладётся в кольцо без `closeRings()`. Цикл `TABRegion::ReadGeometryFromMIFFile`, `addRingDirectly` около строки 1087. И внешнее кольцо, и внутренние проходят через этот цикл, так что один вызов закрывает каждое. `OGRLinearRing::closeRings()` в нашем OGR есть (`ogr/ogrlinearring.cpp`): при различии первой и последней точки добавляет копию первой. Коммит `1a9fbce231`, ticket 5614. Вызов ставить непосредственно перед `addRingDirectly`.

При `numLineSections == 0` указатель геометрии остаётся пустым, и код вызывает `getEnvelope`. Коммит `bc34cd533c`, ticket 6273.

```1113:1116:mitab/mitab_feature_mif.cpp
    SetGeometryDirectly(poGeometry);
    poGeometry->getEnvelope(&sEnvelope);

    SetMBR(sEnvelope.MinX, sEnvelope.MinY, sEnvelope.MaxX, sEnvelope.MaxY);
```

Отрицательное `numLineSections` у Region из MIF ещё и не выделяет массив (`if (numLineSections > 0)`), но затем всё равно доходит до `getEnvelope`. Нулевая и отрицательная секция закрываются одной проверкой `if (poGeometry)` вокруг `getEnvelope` / `SetGeometryDirectly` / `SetMBR`, как в коммите GDAL.

ROUNDRECT с радиусом на следующей строке: условие перевёрнуто (`!= 1` вместо `== 1`), читается токен `[1]`, а не `[0]`. Пустой список токенов читает за концом массива. Коммит `62d8f0bb3d`, ticket 6273. `CSLTokenizeString2(NULL)` в нашем CPL не падает (цикл не входит в тело), `CSLCount(NULL)` возвращает 0. После правки радиус берётся только при ровно одном токене, из `[0]`. Пустая строка, `GetLine() == NULL` и строка из двух и более токенов оставляют радиус 0 и не индексируют `[1]`. Ветка из шести токенов на строке `ROUNDRECT` (`papszToken[5]`) уже верная, её не менять.

```1298:1299:mitab/mitab_feature_mif.cpp
            if (CSLCount(papszToken) !=1 )
              m_dRoundXRadius = m_dRoundYRadius = atof(papszToken[1])/2.0;
```

`TABPolyline::ReadGeometryFromMIFFile` вызывает `fp->GetLine()` и сразу `CSLTokenizeString2` / `atof(papszToken[0])` без проверки пустой строки и числа токенов. Около строк 850–889 для `PLINE MULTIPLE` и простой `PLINE`. Коммит `588cc86485` добавляет отказ, если строка кончилась или в ней не две координаты, и освобождает уже созданные `OGRLineString` / `OGRMultiLineString`.

`TABMAPIndexBlock::PickSeedsForSplit` делит на ширину и высоту экстента без проверки нуля. `mitab_mapindexblock.cpp` около строк 1044–1055. Коммит `2c5c3db03f`, ticket 5256: при `nSrcWidth == 0` или `nSrcHeight == 0` нормализованный зазор равен 0.

`TABMAPObjPLine::ReadObj` кладёт `ReadInt16()` или `ReadInt32()` в `m_numLineSections` и не проверяет знак. `mitab_mapobjectblock.cpp` около строк 961–989. Дальше `TABPolyline` / `TABRegion` делают `CPLMalloc(numLineSections * sizeof(TABMAPCoordSecHdr))` (`mitab_feature.cpp` около 2243 и 3061). Отрицательное значение даёт огромное выделение или падение. Коммит `6edba1e791` возвращает ошибку при `m_numLineSections < 0` сразу после чтения.

Пустой MID после успешного разбора заголовка MIF закрывает файл. `MIFFile::Open`, `mitab_miffile.cpp` около строк 421–430:

```421:430:mitab/mitab_miffile.cpp
    /* Put the MID file at the correct location, on the first feature */
    if (m_eAccessMode == TABRead && (m_poMIDFile->GetLine() == NULL))
    {
        Close();
        ...
        return -1;
    }
```

Коммит `aadd901d14`, ticket 5570, заводит признак «заголовок прочитан и данных нет» из `ParseMIFHeader` и не считает такой конец файла порчей. Сигнатуру `ParseMIFHeader()` в `mitab.h` лучше не менять: достаточно внутреннего флага. Это C++ заголовок библиотеки; публичный C API (`mitab_capi.h`) не трогать.

Сжатые координаты складываются как `m_nComprOrgX + ReadInt16()` и `m_nCenterX + ReadInt16()`. `TABMAPCoordBlock::ReadIntCoord` / `ReadIntCoords`, `mitab_mapcoordblock.cpp` около строк 341–344 и 384–385. `TABMAPObjectBlock::ReadIntCoord`, `mitab_mapobjectblock.cpp` около строк 407–408. Там же label и MBR: `m_nLabelX += m_nComprOrgX` около строки 1009 и аналогичные сложения у Region/Multipoint/Collection в `mitab_feature.cpp` (около 6511 и блок коллекции). Коммиты `65c8807e30`, `105675648f`, `47a3f044c4`, `c6137da5f3`. В GDAL это `TABSaturatedAdd` в `mitab_utils.cpp`: при выходе за `GInt32` значение зажимается в `min`/`max`, отдельная ветка на `nAdd == INT_MIN`. Рядом есть `TABInt16Diff`. Перенос — новая функция в наших `mitab_utils.cpp` / `mitab_utils.h` и замена только этих сложений. Зависимостей GDAL нет, `<limits>` есть в C++11.

`TABMAPCoordBlock::ReadCoordSecHdrs` (`mitab_mapcoordblock.cpp` около строк 433–484) умножает `28 * numSections` или `24 * numSections` и прибавляет `numVertices` без проверки знака и верхней границы. Коммиты `ada8f68602` и `b88156a24b`. В GDAL: отказ, если `numSections` не лезет в `int`, если `numVertices < 0` или не лезет в размер вершины, если `numHoles < 0`.

## 23 кандидата

Приоритет «первая группа» — маленькие правки только в `mapgears/mitab/`, C++11, без новых типов OGR. «Вторая» — полезно, но шире или требует сверки поведения. «Позже» — упирается в отсутствующий `SetFeature`. «Не переносить как есть» — изменение GDAL само по себе не является исправлением нашего кода.

| № | Приоритет | Файл / функция | Исправление | Коммит GDAL | Сложность | Риск |
|---|---|---|---|---|---|---|
| 1 | первая группа | `mitab_coordsys.cpp` case 13 около строки 528; `mitab_spatialref.cpp` case 13 около строки 791 | `break` после Mollweide, иначе проекция 13 читается как Eckert IV | `a5cbb6c535` | низкая | низкий |
| 2 | первая группа | `TABMAPHeaderBlock::Int2Coordsys`; новый `UpdatePrecision`; поля `m_XPrecision` / `m_YPrecision` в закрытой части `TABMAPHeaderBlock` в `mitab_priv.h` | Округление прочитанных координат до точности масштаба. Вызов `UpdatePrecision` из `InitBlockFromData` и `SetCoordsysBounds` | `2c00f6704a` | низкая | низкий |
| 3 | первая группа | `TABFeature::UpdateMBR` в `mitab_feature.cpp` сразу после двух `Coordsys2Int` (около 743) | Поменять местами целые min/max, если квадрант их перевернул | `2789405d5b` | низкая | низкий |
| 4 | первая группа | `TABRegion::ReadGeometryFromMIFFile`, перед `addRingDirectly` (около 1087) | `poRing->closeRings()` | `1a9fbce231` | низкая | низкий |
| 5 | первая группа | тот же метод, `getEnvelope` около 1114 | Не вызывать `getEnvelope`, если геометрия не создана (`Region 0` и отрицательное число секций) | `bc34cd533c` | низкая | низкий |
| 6 | первая группа | `TABRectangle::ReadGeometryFromMIFFile` около 1298 | Радиус со следующей строки: условие `== 1`, токен `[0]`. Пустая строка и лишние токены радиус не читают | `62d8f0bb3d` | низкая | низкий |
| 7 | первая группа | `TABPolyline::ReadGeometryFromMIFFile` около 850–889 | Отказ, если `GetLine()` пустой или в строке не две координаты; освободить уже созданные линии | `588cc86485` | низкая | низкий |
| 8 | первая группа | `TABMAPIndexBlock::PickSeedsForSplit` около 1054 | При нулевой ширине или высоте нормализованный зазор = 0 | `2c5c3db03f` | низкая | низкий |
| 9 | первая группа | `TABMAPObjPLine::ReadObj` после чтения числа секций, около 988 | Ошибка и `-1`, если `m_numLineSections < 0`, до любого `CPLMalloc` | `6edba1e791` | низкая | низкий |
| 10 | первая группа | `MIFFile::Open` около 422 и `ParseMIFHeader` | Конец файла сразу после заголовка — слой из 0 объектов. Сигнатуру в `mitab.h` не менять, флаг оставить внутренним | `aadd901d14` | низкая | низкий |
| 11 | вторая | `ReadIntCoord` / `ReadIntCoords` в `mitab_mapcoordblock.cpp` и `mitab_mapobjectblock.cpp`; сложения label/MBR в object block и `mitab_feature.cpp` | Насыщенное сложение `GInt32` вместо переполнения на сжатых координатах у границы ±1e9 | `65c8807e30`, `105675648f`, `47a3f044c4`, `c6137da5f3` | средняя | низкий |
| 12 | вторая | `TABMAPCoordBlock::ReadCoordSecHdrs` | Отказ при отрицательном числе вершин или дыр и при переполнении размера заголовка секции | `b88156a24b`, `ada8f68602` | средняя | низкий |
| 13 | вторая | разбор заголовка `TABDATFile` | Проверка полей заголовка `.dat`: переполнение `int32` и деление на ноль | `5382e0b103` | средняя | низкий |
| 14 | вторая | `TABMAPObjCollection::ReadObj`; чтение текста и multipoint в `mitab_feature.cpp` и `mitab_mapobjectblock.cpp` | Предел длины текста, числа точек и суммы секций коллекции до выделения памяти | `a7bb64403a`, `427aa80e72` | средняя | низкий |
| 15 | вторая | `TABINDFile::BuildKey` для float, около `CPLAssert(nKeyLength == 8)`; вызовы из `mitab_tabview.cpp` | Утверждение заменить на ошибку. Пустой ключ не передавать в `FindFirst`. `std::unique_ptr` из того же коммита GDAL не переносить | `3c54bad0d9` | низкая | низкий |
| 16 | вторая | обход дерева `TABINDFile` | Предел глубины на битом `.ind`, чтобы не уйти в бесконечную рекурсию и не переполнить стековый буфер ключа | `e42cbe72c2`, `f6f5f56776` | средняя | средний |
| 17 | вторая | `MIDDATAFile`, разбор записи `.mid`, связанный код в `mitab_miffile.cpp` / `mitab_feature_mif.cpp` | Кавычки и перевод строки внутри строкового поля. Длинные строки MID/MIF до порядка 1 МБ. Смотреть diff целиком: коммит длинных строк меняет размер буфера | `67efcedf63`, `2834e3953c` | средняя | средний |
| 18 | вторая | `TAB_CSLLoad` в `mitab_utils.cpp`; чтение MIF в `mitab_miffile.cpp` / `mitab_middatafile.cpp` | Убрать квадратичное чтение заголовка. Ограничить работу на заведомо битом MIF | `2b4a33db40`, `7355f684f4` | средняя | низкий |
| 19 | вторая | разбор ширины пера в `mitab_feature.cpp` | Поддержать `px` и дробные `pt`. Сверять с текущим разбором style string, не копировать файл целиком | `d822550fbc` | средняя | средний |
| 20 | вторая | `mitab_spatialref.cpp`, `mitab_coordsys.cpp`, таблица charset в `mitab_imapinfofile.cpp` | Единицы не в метрах; LCC_2SP с неметровой единицей; поперечная Меркатора KKJ; датум GDA2020; charset WindowsBalticRim / CP1257. Логика сейчас в `ogr\ogrmitabspatialref.cpp` GDAL, править наши два файла CRS | `95a0f6c897`, `70f1cec420`, `51e1392643`, `b774a0a52d`, `5474db1f63` | средняя | средний |
| 21 | вторая | расчёт bounds при создании слоя, `mitab_ogr_datasource.cpp` | Сдвинуть bounds по умолчанию под false easting / false northing, чтобы внутренние координаты не упирались в ±1e9 | `a267d195f4` | средняя | средний |
| 22 | позже | `TABMAPFile`: обновление объекта, цепочка coordinate block, файлы без index block | Три связанных исправления записи существующего объекта. У нас `SetFeature` не реализован, поэтому патчи сами по себе не к чему применить | `a9cc66f748`, `8037e7d9ca`, `63f317b293` | высокая | высокий |
| 23 | не переносить как есть | `IMapInfoFile::CreateFeature` в `mitab_imapinfofile.cpp`; кусок `TABFile::SetFeature` в том же коммите | GDAL стал возвращать `OGRERR_NONE`, если преобразование коллекции дало `NULL`, и отдельно запретил `SetFeature` для MultiPoint и GeometryCollection. У нас создание коллекции уже идёт своим путём (`mitab_imapinfofile.cpp`, комментарий про коллекции). Слепое копирование спрячет ошибку преобразования | `4ab0e98d4f` | — | высокий |

Рядом с пунктами 11–16 лежат однотипные проверки битых файлов. Их брать отдельными маленькими переносами, сверяя функцию, а не файл:

- `f0d1b6358c` — `GetCoordBlock()` может вернуть `NULL`, чтение геометрии в `mitab_feature.cpp` должно это пережить.
- `3bd029849e` — верхний предел памяти при чтении битой геометрии MIF в `mitab_feature_mif.cpp`.
- `08ce0febbb` — утечки в ошибочных ветках `TABMAPFile::SplitObjBlock`.
- `c65bf13dca` — бесконечный цикл в ошибочной ветке `mitab_mapfile.cpp`.
- `48824454f3` — переполнение `int` в `mitab_mapindexblock.cpp` (OSS-Fuzz 7697).
- `96a50b61b9` — переполнение `int32` в `TABIDFile::Open`.
- `94bd237832` — переполнение при расчёте bounding box в object block.
- `30995f877e` — открытие `.mif`, у которого в заголовке есть колонки, а `.mid` нет (ticket 5141). Текст коммита ошибочно говорит «без .mif».
- `37f2abd9d8` — `CreateFeature` не должен падать на битой строке стиля OGR (ticket 1209).
- `fd5a52b3fb` — запись `.tab`, если в имени поля есть недопустимые символы (GitHub #924). Diff коммита задевает много файлов из-за попутных правок; переносить только обработку имени.
- `e3e143449b` — неинициализированные поля, из-за которых плавал тест обновления.
- `458cc8f13d` — соответствие номеров символов MapInfo и идентификаторов стиля OGR.
- `1a84c30724` — флаг режима обновления в datasource (GitHub #2170). Это поведение OGR-обёртки, не ядро формата.
- `7f18ca5ecc`, `b194294df4`, `aca98778af` — утечка и `NULL` в ошибочных ветках `TABDATFile`.

Их нет в нумерованной двадцать тройке. Журнал `PROGRESS.md` ведёт только эти 23 пункта. Дополнительные SHA брать в работу только отдельным решением и новой строкой журнала.

## План проверки по каждому пункту

Сборка для всех пунктов, где меняется код: `Release|Win32`, toolset `v120_xp`, решение `MITAB.sln` / `MITAB.vcxproj`. Публичный C API, `cpl/`, `ogr/` и файлы проектов не менять. После каждой порции смотреть `git diff` и убеждаться, что посторонних файлов нет. Готового набора autotest GDAL в нашем дереве нет. Проверки ниже — чтение и запись маленьких TAB/MIF через библиотеку (`tab2tab` или короткий вызов API). Если образца файла нет, это нужно прямо записать в журнал, а не считать пункт проверенным.

| № | Как проверить |
|---|---|
| 1 | MIF и TAB с кодом проекции 13. Имя проекции OGR — Mollweide. Eckert IV не выставляется. Проекция 14 по-прежнему Eckert IV. Прогнать и `MITABCoordSys2SpatialRef` (MIF), и `TABFile::GetSpatialRef` (TAB) |
| 2 | Запись и чтение координаты с bounds, как в mapgears/mitab#2. Значение возвращается без хвоста двоичного округления (`...0002`). Файл, созданный старой библиотекой, продолжает читаться |
| 3 | Для квадрантов 1, 2, 3, 4 и 0 создать объект с ненулевым экстентом. Целый MBR удовлетворяет `m_nXMin <= m_nXMax` и `m_nYMin <= m_nYMax`. Объект читается обратно в тех же мировых границах. Квадрант 1 не должен менять уже правильный порядок |
| 4 | MIF `Region` с незамкнутым внешним кольцом и незамкнутым внутренним. После чтения первая и последняя точки каждого кольца совпадают. Уже замкнутое кольцо не получает лишнюю точку |
| 5 | MIF `Region 0` открывается, геометрии нет, процесса не рвёт. `Region` с одной нормальной секцией читается как раньше |
| 6 | Три файла: `ROUNDRECT` с радиусом шестым токеном строки; радиус на следующей строке одним числом; пустая следующая строка и строка из двух токенов. Первый и второй дают ненулевой радиус (в файле радиус делится на 2, как и раньше). Третий и четвёртый не читают `[1]` и не падают; радиус остаётся 0, рисуется обычный прямоугольник |
| 7 | Обрезанный `PLINE` и `PLINE MULTIPLE` (нет строки с числом вершин, нет строки координаты, в строке один токен) возвращают ошибку и не оставляют утечки. Полный `PLINE` читается |
| 8 | Запись набора объектов, у которых экстент по X или по Y вырожден, так что индекс вынужден делить узлы. Деления на ноль нет, файл открывается |
| 9 | Объект Region/PLine, у которого поле числа секций отрицательное. Функция возвращает ошибку, `CPLMalloc` с этим числом не вызывается. Обычный регион из одной секции читается |
| 10 | MIF с `Columns` и без записей, парный MID пустой. `Open` успешен, число объектов 0. MIF с одной записью по-прежнему читает первую строку MID |
| 11 | Сжатый объект, у которого начало сжатия близко к ±1000000000 и дельта `int16` выталкивает сумму за пределы `GInt32`. Координата зажимается, процесс не попадает в неопределённое поведение знакового переполнения. Обычные сжатые точки в середине диапазона не меняются |
| 12 | Секция с отрицательным `numVertices` или `numHoles` и секция с огромным `numSections`. Чтение завершается ошибкой. Нормальный регион V450/V800 читается |
| 13 | `.dat` с нулевой длиной записи или несогласованным заголовком. Открытие даёт ошибку, деления на ноль нет. Обычный `.dat` читает те же значения полей |
| 14 | Завышенная длина текста и число точек multipoint в заголовке объекта. Память под них не выделяется «сколько попросили». Нормальные text и multipoint читаются |
| 15 | `.ind` с длиной float-ключа не 8. Ошибка вместо `CPLAssert`. Обычный числовой индекс ищет запись |
| 16 | `.ind` с циклом в указателях поддерева и с порванным ключом. Обход останавливается, стек не растёт без предела. Исправный индекс по-прежнему находит ключ |
| 17 | MID, где внутри кавычек есть перевод строки. Поле читается целиком, следующая запись не съезжает. Строка длиннее старого фиксированного буфера читается до предела около 1 МБ и не портит соседнее поле |
| 18 | MIF с очень большим числом строк до `DATA` заканчивается за приемлемое время. Заведомо битый MIF не крутит разбор до бесконечности. Маленький нормальный MIF читается как раньше |
| 19 | Стиль пера `px` и дробный `pt`. Ширина в TAB совпадает с тем, что пишет GDAL на тех же образцах из коммита. Целые `pt`, которые мы пишем сейчас, не меняются |
| 20 | Отдельный образец на каждый подпункт: линейная единица не метр; LCC с двумя стандартными параллелями и неметровой единицей; TM на KKJ; датум GDA2020; charset CP1257 / WindowsBalticRim. Имя проекции, единицы и код charset совпадают с ожидаемыми. Соседние коды проекций не сдвигаются |
| 21 | Слой с большим false easting/northing. Bounds по умолчанию сдвигаются, запись точки в рабочей зоне не включает предупреждение integer bounds overflow. Слой без ложного сдвига сохраняет прежние bounds |
| 22 | Только после появления рабочего `SetFeature`: правка геометрии, координаты которой занимают несколько coordinate block; TAB от MapInfo с одним объектом и без index block; TAB без index block, но с coordinate block. После правки объект читается, соседние объекты не портятся. Пока `SetFeature` возвращает `OGRERR_UNSUPPORTED_OPERATION`, пункт не реализовывать |
| 23 | Не переносить механически. Если когда-нибудь понадобится, отдельно сравнить наш `IMapInfoFile::CreateFeature` для `wkbMultiPoint` и `wkbGeometryCollection` с diff `4ab0e98d4f` и решить, ошибка это или успех. До этого решения код не менять |

Первая порция, которую имеет смысл делать одним заходом: пункты 1–10. Каждый остаётся в `mapgears/mitab/`.

## Что нельзя перенести без CPL или OGR

Эти изменения в двадцать три пункта не входят. Статус для них — BLOCKED, пока нет отдельного разрешения менять `cpl/` или `ogr/`.

| Тема | SHA | Почему не входит в `mitab/` |
|---|---|---|
| LargeInt / Integer64 | `d6efa295ac` | В нашем `ogr/ogr_core.h` нет `OFTInteger64`. Тип поля — публичный OGR API. Бинарный тип в `.dat` без нового кода OGR наружу не отдать |
| UTF-8 и перекодирование подписей | `294298d34a`, `3340eefbe6`, `bec881f81d`, `7cc2b29b79`, `4235dd66b9` | Рабочий путь GDAL — `CPLRecode`. У нас `cpl/cpl_recode_stub.cpp` перекодирования не делает. Таблицу CP1257 (часть пункта 20, `5474db1f63`) можно добавить внутри `mitab/`. Полноценный UTF-8 — нет |
| Датум из WKT2 | `0eaf309509` | Нужен современный разбор WKT в OGR |
| `AlterFieldDefn` и порча Integer ширины 4 | `d5cd568605` | Виртуального `AlterFieldDefn` у нашего `OGRLayer` нет. Бага в этом API у нас нет |
| Методы проекции 34 и 35 | `a0bc327d2f`, `4529e16721` | Имеет смысл только если хватает уже существующих `SetTM` / `SetHOM` / `SetTMVariant`. Новый `SRS_PT_*` — правка `ogr/` |
| `TestCapability()` возвращает `bool` | `8d47a94075` | Ломает наш C++ API слоя |
| `ICreateLayer` с `const OGRSpatialReference*` и точностью координат | `4b1070084c`, `0c92a7fc29` | Сигнатура OGR, которой у нас нет |
| Вынос драйвера в плагин, Northwood без TAB | `28a00b0825`, `b7830458db` | Сборка GDAL, не библиотека MITAB |
| Метаданные `BOUNDS` | `c63d599807` | Новая возможность OGR metadata, не исправление чтения. При желании делается отдельно и только если старый `SetMetadataItem` это позволяет без правки `ogr/` |

## Что не считать улучшением

- Замена `NULL` на `nullptr`, `final`, `override`, `std::unique_ptr` / `std::make_unique` по файлам драйвера.
- Предупреждения cppcheck, Coverity и `-Wold-style-cast`, если условие и так не меняет результат на наших компиляторах. Исключение — пункты, где предупреждение совпало с реальной утечкой или переполнением и SHA уже указан выше.
- Перенос файлов GDAL целиком. Даже `mitab_feature.cpp` отличается форматированием и типами (`GIntBig`, `CPLStringList`, `std::unique_ptr`).
- Правка `cpl/`, `ogr/`, `*.vcxproj`, `*.sln`, `CMakeLists.txt` ради того, чтобы кусок GDAL скомпилировался.
- Возможности C++ новее C++11. Toolset `v120_xp`, Win32, с прицелом на x64 (`GInt32` не заменять на `long`, размеры через `size_t` там, где GDAL уже проверяет переполнение).

## Как пользоваться документом в следующей сессии

1. Прочитать этот файл и `docs/gdal-migration/PROGRESS.md`.
2. Взять один номер из журнала со статусом `TODO`.
3. Открыть только перечисленные SHA: `git show <sha> -- "gdal/ogr/ogrsf_frmts/mitab"` или `ogr/ogrsf_frmts/mitab` в клоне GDAL. Для CRS после 2024 года смотреть `ogr/ogrmitabspatialref.cpp`, править наши `mitab_spatialref.cpp` и `mitab_coordsys.cpp`.
4. Сверить функцию в `mapgears/mitab/` с diff коммита. Если наш код уже совпадает с исправлением, пункт не переносить, в журнале указать, где это видно.
5. Сборку и тесты записать в журнал так, как они реально прошли. Не ставить `VERIFIED`, если Win32 `v120_xp` не собирался или проверка из таблицы выше не выполнялась.
