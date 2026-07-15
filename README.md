# hide

MetaMod-плагин для **Counter-Strike 2**: скрытый режим наблюдателя для админов.

Админ пишет в чат `!hide` — и пропадает с сервера для всех остальных: его нет в TAB-таблице, его не видно на карте, при этом он сам продолжает всё видеть, свободно летая в спектаторе.

Ядро (загрузка схем, gamedata, хуки) адаптировано из [CS2Fixes](https://github.com/Source2ZE/CS2Fixes) от Source2ZE.

---

## Что делает

| Команда | Кто может | Действие |
|---|---|---|
| `!hide` / `/hide` | админ | Включает/выключает скрытие. При включении админ переводится в спектаторы. |
| `!hide_reload` | админ | Перечитывает `admins.json` без перезапуска сервера. |

Поведение:

- **Невидимость в TAB.** Контроллер и pawn скрытого админа вырезаются из снапшота (`CheckTransmit`) для всех клиентов, кроме него самого. Клиент физически не получает данных — значит, ни в скорборде, ни на радаре, ни в мире админа нет.
- **Своя строчка в TAB тоже скрывается.** Собственному клиенту контроллер вырезать нельзя (чёрный экран), поэтому скрытый админ помечается сетевым флагом `m_bIsHLTV` — скорборд отфильтровывает его как GOTV-клиента.
- **Другие скрытые админы тоже не видят друг друга** — каждый видит только себя.
- **Автоперевод в спектаторы.** При `!hide` админ уходит в `CS_TEAM_SPECTATOR`.
- **Защита от посторонней логики.** Раз в ~секунду (каждый 64-й тик) проверяется, не закинуло ли скрытого админа обратно в команду (ребаланс, события раунда, другие плагины) — если да, он возвращается в спектаторы.
- **Выключение по выбору команды.** Если админ сам жмёт `jointeam` (любая команда, включая спектаторов), скрытие снимается — плагин не спорит с осознанным выбором игрока.
- **Скрытие живёт в рамках сессии.** При дисконнекте плагин забывает игрока: реконнект — всегда чистый заход обычным видимым игроком, скрытие включается только явным `!hide`.
- **Команда не светится в чате.** Сам текст `!hide` подавляется (`MRES_SUPERCEDE`), обычные игроки его не видят.

Для не-админов `!hide` — просто обычное сообщение в чат.

---

## Установка

1. Собери плагин (см. [Сборка](#сборка)) или возьми готовый архив.
2. Скопируй содержимое в корень сервера — получится:

```
csgo/addons/hide/bin/linux64/hide.so
csgo/addons/hide/configs/admins.json
csgo/addons/hide/gamedata/hide.jsonc
csgo/addons/metamod/hide.vdf
```

3. Создай `admins.json` из примера:

```bash
cp csgo/addons/hide/configs/admins.json.example \
   csgo/addons/hide/configs/admins.json
```

Требуется установленный [MetaMod:Source](https://www.sourcemm.net/) для CS2.

---

## Конфигурация

### admins.json

Объект, где **ключ — SteamID64**. Содержимое значений плагин не читает — важны только ключи:

```json
{
	"76561198724970203": {
		"name": "admin name",
		"immunity": 100,
		"permissions": null,
		"groups": null
	}
}
```

Файл ищется по двум путям (берётся первый существующий):

1. `csgo/addons/hide/configs/admins.json`
2. `csgo/addons/counterstrikesharp/configs/plugins/AdminPlugin/admins.json`

Второй путь — совместимость с CounterStrikeSharp: если у тебя уже есть админы там, отдельный конфиг заводить не надо.

Список перечитывается при смене карты и по `!hide_reload`. Комментарии в JSON допускаются, ошибки парсинга не роняют сервер.

### gamedata/hide.jsonc

Смещения и индексы vtable. **Могут потребовать обновления после крупных патчей CS2.** Сверяться с [cs2fixes.jsonc](https://github.com/Source2ZE/CS2Fixes/blob/main/gamedata/cs2fixes.jsonc).

| Ключ | Что это |
|---|---|
| `GameEntitySystem` | Смещение `CGameEntitySystem*` внутри `CGameResourceService` |
| `CCSPlayerController_ChangeTeam` | Индекс vtable у `ChangeTeam(int)` |
| `CheckTransmitPlayerSlot` | Смещение слота игрока в `CCheckTransmitInfo` |

---

## Сборка

Сборка идёт в Docker на образе SteamRT Sniper — ничего ставить локально не нужно.

```bash
docker build -t hide-build .
docker run --rm -v "$PWD":/app/source hide-build
```

Результат: `dockerbuild/package/` — готовая к копированию на сервер структура.

Образ подтягивает всё сам: [AMBuild](https://github.com/alliedmodders/ambuild), [metamod-source](https://github.com/alliedmodders/metamod-source), [hl2sdk (ветка cs2)](https://github.com/alliedmodders/hl2sdk), clang-21.

### Ручная сборка

Нужны `HL2SDKCS2`, `MMSOURCE_DEV`, `HL2SDKMANIFESTS` в окружении:

```bash
mkdir -p build && cd build
python ../configure.py --enable-optimize --sdks cs2
ambuild
```

---

## Тесты

Юнит-тесты на парсинг `admins.json` и SteamID64:

```bash
docker run --rm -v "$PWD":/app/source hide-build bash tests/run_tests.sh
```

---

## Структура

```
src/
  hide.cpp/h          Точка входа, MetaMod-хуки, логика скрытия
  adminlist.cpp/h     Загрузка и парсинг admins.json
  gameconfig.cpp/h    Чтение gamedata (смещения, vtable)
  cs2_sdk/            Схемы и обёртки над сущностями CS2
  utils/              Хелперы для вызова виртуальных методов
gamedata/hide.jsonc   Смещения под текущую версию CS2
configs/              Пример admins.json
tests/                Юнит-тесты
vendor/nlohmann/      JSON-библиотека (header-only)
```

Ключевые хуки в [src/hide.cpp](src/hide.cpp):

| Хук | Зачем |
|---|---|
| `CheckTransmit` | Собственно невидимость — вырезание из снапшота |
| `DispatchConCommand` | Ловля `!hide` в чате |
| `ClientCommand` | Отслеживание `jointeam` |
| `ClientDisconnect` | Сброс скрытия при выходе (сессионная семантика) |
| `StartupServer` | Перезагрузка админов на смене карты |
| `GameFrame` | Раз в секунду возвращает скрытых в спектаторы и переустанавливает HLTV-флаг |

---

## Лицензия

GPL v3. Ядро адаптировано из [CS2Fixes](https://github.com/Source2ZE/CS2Fixes) (© 2023–2026 Source2ZE), плагин © 2026 2kx.
