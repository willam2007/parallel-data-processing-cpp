# Параллельная обработка данных: C++20 и OpenMP

Один репозиторий для практических занятий магистратуры. Основной рабочий
редактор — VS Code. На Windows используются MinGW-w64 GCC и GDB из MSYS2
UCRT64. На Linux — GCC, на macOS — Homebrew LLVM и libomp.

**Другому агенту:** сначала прочитай [AGENTS.md](AGENTS.md). Там есть маршрут
развёртывания, точные команды проверки и ограничения проекта.

| Занятие | Файл | Содержание |
| --- | --- | --- |
| 1, 28.09.2026 | [main.cpp](main.cpp) | Порядок сложения, точность double, OpenMP reduction |
| 2, исходники 12.10.2026 | [practice02.cpp](practice02.cpp) | Частичные суммы, ложное разделение кеша, блоки, дерево |
| Подготовка к 3, 26.10.2026 | [practice03.cpp](practice03.cpp) | Сумма матрицы, A+B, A*x, A*B; точное задание преподавателя пока неизвестно |

Разбор второго занятия: [materials/practice02](materials/practice02/README.md).
Подготовка и теория матриц: [materials/practice03](materials/practice03/README.md).

## Быстрый старт на Windows, если MSYS2 уже установлен

Из папки репозитория в обычном PowerShell:

```powershell
powershell -NoProfile -ExecutionPolicy Bypass -File scripts/setup-windows.ps1
powershell -NoProfile -ExecutionPolicy Bypass -File scripts/build.ps1 -Test
powershell -NoProfile -ExecutionPolicy Bypass -File scripts/run.ps1 -Practice 3 -Task 2
code .
```

`setup-windows.ps1` устанавливает недостающие пакеты GCC, GDB, CMake и Ninja
в существующий MSYS2 и расширение Microsoft C/C++, если команда `code` доступна.
`build.ps1 -Test` собирает все три программы и выполняет тесты.
`run.ps1` сначала собирает выбранную программу, затем запускает её.
Скрипты не меняют постоянную политику выполнения PowerShell.

## Windows с нуля

### 1. Инструменты

На обычной 64-битной Windows x64 установи [Git](https://git-scm.com/download/win),
[VS Code](https://code.visualstudio.com/) и [MSYS2](https://www.msys2.org/).
Для Windows x64 выбирай установщик MSYS2 x86_64, установку — в `C:\msys64`.
Можно использовать winget:

```powershell
winget install --id Git.Git --exact
winget install --id Microsoft.VisualStudioCode --exact
winget install --id MSYS2.MSYS2 --exact
```

Закрой и снова открой PowerShell после установки, чтобы обновился PATH.
Windows ARM64 требует соответствующих инструментов; для этого маршрута проще
использовать Ubuntu ARM64 в WSL и раздел Linux ниже. UCRT64 — среда x86_64,
а не нативный компилятор ARM64. MSVC в этом проекте не настроен.

### 2. Обновление MSYS2

Открой **MSYS2 UCRT64** из меню «Пуск» и выполни:

```bash
pacman -Syu
```

Если обновление попросит закрыть терминал, закрой его, открой UCRT64 заново
и повтори команду. Затем можно закрыть MSYS2 и вернуться в PowerShell.
При автоматическом развёртывании агент может вызвать `usr/bin/bash.exe -lc`
с `pacman -Syu --noconfirm`; после обновления среды может понадобиться новый
процесс bash и повтор команды. Ошибки обновления нельзя игнорировать.

### 3. Получение репозитория

Рекомендуемый путь без кириллицы:

```powershell
git clone https://github.com/willam2007/parallel-data-processing-cpp.git C:\CppProjects\parallel-data-processing-cpp
cd C:\CppProjects\parallel-data-processing-cpp
```

Для приватного репозитория предварительно войди в GitHub. Например, установи
GitHub CLI (`winget install --id GitHub.cli --exact`), выполни `gh auth login`
и клонируй через `gh repo clone willam2007/parallel-data-processing-cpp`.
Доступ другого агента обеспечивается его авторизованным аккаунтом; токены
и персональные настройки в репозитории не хранятся. Вместо Git можно скачать
архив исходников на GitHub и распаковать его в каталог без кириллицы.

### 4. Подготовка и проверка

Из корня проекта выполни команды быстрого старта выше. Они рассчитаны на
PowerShell 5.1 и новее. При удачной проверке CTest выводит `100% tests passed`.
CMake должен найти OpenMP; предупреждение об игнорируемой pragma или ошибка
`undefined reference to omp_get_wtime` означают неправильную сборку.

### 5. Запуск через F5

1. Открой всю папку репозитория в VS Code: `code .`.
2. Установи **C/C++ от Microsoft**, идентификатор `ms-vscode.cpptools`.
3. Нажми **Ctrl+Shift+D** и сверху выбери нужную **Практика 1/2/3 ... (GDB)**.
4. Нажми **F5**. Задача сначала соберёт нужный исполняемый файл через CMake.
5. Вывод и ввод находятся во вкладке **Терминал**. Остановить — **Shift+F5**.

Для первых двух занятий F5 передаёт `--size 1048576` — массив всего 8 МиБ.
Чтобы повторить исходный эксперимент с округлением и 2 ГиБ памяти,
в `.vscode/launch.json` поменяй размер на `268435456` или используй команду ниже.
У третьей практики F5 использует настройки в её `main()`.
Открытая вкладка не определяет запускаемую программу: её определяет конфигурация.

Поставить точку останова — **F9**, выполнить строку целиком — **F10**.
**Ctrl+F5** запускает выбранную конфигурацию без отладки.
**Ctrl+Shift+B** по умолчанию собирает первое занятие; другие задачи доступны
через **Терминал → Выполнить задачу**.

## MSYS2 в другой папке

Для PowerShell-скриптов укажи bin выбранной среды:

```powershell
powershell -NoProfile -ExecutionPolicy Bypass -File scripts/setup-windows.ps1 -MsysRoot D:\msys64
$env:CPP_TOOLCHAIN_BIN = 'D:\msys64\ucrt64\bin'
powershell -NoProfile -ExecutionPolicy Bypass -File scripts/build.ps1 -Test
```

Также можно передавать `-ToolchainBin D:\msys64\ucrt64\bin` скриптам build/run.
Для F5 поменяй `course.toolchainBin` в `.vscode/settings.json`.
Для терминала поменяй там же `terminal.integrated.env.windows.PATH`,
а для подсказок до первой сборки — Win32 `compilerPath` в
`.vscode/c_cpp_properties.json`. После сборки подсказки получают точные флаги
из `build/debug/compile_commands.json`.
Системный PATH для скриптов и F5 менять не обязательно.

## Ubuntu / Debian / WSL

```bash
sudo apt-get update
sudo apt-get install -y git g++ gdb cmake ninja-build
git clone https://github.com/willam2007/parallel-data-processing-cpp.git
cd parallel-data-processing-cpp
bash scripts/build.sh debug all
ctest --test-dir build/debug --output-on-failure
./build/debug/practice01 --size 1048576
./build/debug/practice02 --size 1048576
./build/debug/practice03 --task 2 --size 2048 --repeats 5
```

Для других Linux установи соответствующие пакеты GCC с поддержкой C++20,
OpenMP runtime, GDB, CMake >=3.20 и Ninja. Далее команды одинаковы.
В WSL храни проект в домашнем каталоге Linux и открывай VS Code через WSL,
чтобы компилятор и расширения работали в одной среде.
В VS Code для Linux работают конфигурации GDB с теми же названиями:
tasks.json выбирает bash, launch.json — Linux executable и `/usr/bin/gdb`.
Если GDB установлен иначе, исправь Linux override `miDebuggerPath`.

## macOS, включая Apple Silicon

Установи Command Line Tools и [Homebrew](https://brew.sh/) по официальной инструкции:

```bash
xcode-select --install
brew install git cmake ninja llvm libomp
git clone https://github.com/willam2007/parallel-data-processing-cpp.git
cd parallel-data-processing-cpp
bash scripts/build.sh debug all
ctest --test-dir build/debug --output-on-failure
./build/debug/practice03 --task 2 --size 2048 --repeats 5
```

Скрипт сам определяет Homebrew prefix и использует LLVM `clang++`, а не
системный Apple Clang. libomp предоставляет OpenMP. Пути `/opt/homebrew`
или `/usr/local` вручную вписывать не нужно.
Если CMake не находит omp.h, смотри диагностику OpenMP ниже.

Для F5 установи **CodeLLDB**, идентификатор `vadimcn.vscode-lldb`, и выбери
конфигурацию **Практика ... (macOS, LLDB)**. Её задача сборки вызывает тот же
bash-скрипт. Основные конфигурации GDB предназначены для Windows/Linux.
Для командной строки VS Code не требуется.

## Запуск и выбор размера без редактирования кода

### Windows

```powershell
# Быстрый запуск первой и второй практических: 8 МиБ данных.
powershell -NoProfile -ExecutionPolicy Bypass -File scripts/run.ps1 -Practice 1
powershell -NoProfile -ExecutionPolicy Bypass -File scripts/run.ps1 -Practice 2

# Исходный большой эксперимент: 2 ГиБ только на массив.
powershell -NoProfile -ExecutionPolicy Bypass -File scripts/run.ps1 -Practice 1 -Size 268435456

# Матрицы: 0 — сумма A, 1 — A+B, 2 — A*x, 3 — A*B.
powershell -NoProfile -ExecutionPolicy Bypass -File scripts/run.ps1 -Practice 3 -Task 3 -Size 256 -Repeats 5

# Оптимизированная сборка, без отладчика.
powershell -NoProfile -ExecutionPolicy Bypass -File scripts/run.ps1 -Practice 3 -Task 2 -Configuration Release
```

### Linux / macOS

```bash
bash scripts/build.sh release all
./build/release/practice01 --size 1048576
./build/release/practice02 --size 1048576
./build/release/practice03 --task 3 --size 256 --repeats 5
./build/release/practice03 --help
```

Сами программы принимают `--size N`; третья также принимает `--task 0..3`
и `--repeats N`. В первых двух `--size` — длина массива, в третьей — сторона
квадратной матрицы. Ноль, отрицательные значения и неверные параметры отвергаются.
При запуске первых двух `.exe` напрямую без `--size` сохранён исходный размер
`1 << 28`, поэтому для первого запуска всегда указывай небольшой размер.
Ожидаемая сумма первых двух автоматически пересчитывается по N.

Число потоков можно задать окружением:

```powershell
$env:OMP_NUM_THREADS = '4'
powershell -NoProfile -ExecutionPolicy Bypass -File scripts/run.ps1 -Practice 3 -Task 2
```

```bash
OMP_NUM_THREADS=4 ./build/release/practice03 --task 2 --size 2048
```

`Debug` предназначен для разбора кода, `Release` использует оптимизацию GCC/Clang
`-O2` без `fast-math`. Для сравнения производительности прогрей оба варианта,
сделай несколько запусков без отладчика и учитывай загрузку системы.
Маленькая задача может работать параллельно медленнее. Числа времени не являются тестом.
Параметр CACHE_LINE=64 во второй практике — учебное предположение о CPU;
на другой архитектуре оно может хуже устранять false sharing, не меняя правильность сумм.

## Структура и сборка

```text
main.cpp, practice02.cpp, practice03.cpp  # отдельные программы с main()
include/course_options.h                # необязательные параметры запуска
CMakeLists.txt                          # единая переносимая сборка
scripts/                                # установка, сборка, запуск
tests/checks.cpp                        # проверки алгоритмов на известных ответах
materials/                              # теория и исходники занятий
.vscode/                                # выбор практики и F5
.github/workflows/build.yml              # проверка Windows, Linux, macOS
AGENTS.md                               # инструкции следующему агенту
build/debug/, build/release/             # генерируются, не хранятся в Git
```

Не собирай все `.cpp` через `g++ *.cpp`: у занятий разные `main()`.
CMake создаёт отдельные цели `practice01`, `practice02`, `practice03` и
`course_checks`. Тесты не запускают большие эксперименты: используют маленькие
массивы, матрицы с известными ответами, дробные данные и разное число потоков.
GitHub Actions проверяет сборку и CTest на трёх ОС при push/PR.
Успешный CI не означает, что отладка через UI VS Code проверена на всех этих ОС.

Общая ручная команда после установки инструментов:

```bash
cmake -S . -B build/debug -G Ninja -DCMAKE_BUILD_TYPE=Debug -DCMAKE_CXX_COMPILER=g++
cmake --build build/debug --parallel 2
ctest --test-dir build/debug --output-on-failure
```

На Windows добавь `C:\msys64\ucrt64\bin` в PATH текущего PowerShell перед ручной
командой. На macOS используй scripts/build.sh: он задаёт LLVM и prefix libomp.
OpenMP подключается целью `OpenMP::OpenMP_CXX`, вручную дописывать `-lgomp`
не требуется. Для очень короткого ручного эксперимента можно использовать
`g++ -std=c++20 -fopenmp -g -O0 main.cpp -o practice01`, но обычный маршрут — CMake.

## Как добавить следующую практическую

1. Создай `practice04.cpp` с собственным `main()`.
2. Добавь `course_target(practice04 practice04.cpp)` в CMakeLists.txt.
3. Скопируй задачу сборки и конфигурацию запуска, заменив название цели.
   Добавь новое значение в ValidateSet скриптов PowerShell и список целей build.sh.
4. Добавь маленькие проверки правильности и описание занятия в materials/.
5. Выполни сборку и CTest. Большие замеры запускай отдельно от тестов.

## Если что-то не работает

- **Access denied / repository not found при clone:** проверь авторизацию и доступ
  к приватному репозиторию. Не помещай токен в URL или файл проекта.
- **CMake или Ninja не найдены:** на Windows выполни setup-windows.ps1;
  на Linux/macOS установи пакеты из соответствующего раздела.
- **CMake выбрал MSVC:** используй scripts/build.ps1 с GCC. Старый каталог сборки
  от другого компилятора не переиспользуй; для ручной проверки выбери новый `-B`.
- **Не найден OpenMP:** проверь компилятор GCC/LLVM и установленный runtime.
  На macOS переустанови `llvm libomp`; при ручной настройке можно передать
  `-DOpenMP_CXX_INCLUDE_DIR="$(brew --prefix libomp)/include"` и
  `-DOpenMP_omp_LIBRARY="$(brew --prefix libomp)/lib/libomp.dylib"`.
- **Игнорируется pragma / undefined reference to omp_get_wtime:** запущена другая
  сборка без OpenMP. Выбери конфигурацию практики; не используй сторонний Runner.
- **F5 запускает расширение C/C++ Compile Run:** оно может перехватывать F5.
  Отключи его для этой рабочей области или убери привязку `extension.Debug`
  через редактор сочетаний клавиш. Для проекта достаточно Microsoft C/C++.
- **Не найдена DLL при запуске Windows exe:** запускай через run.ps1/F5 или добавь
  bin того же MSYS2 UCRT64 в PATH текущего процесса.
- **GDB Error creating process / error 2:** открой/клонируй проект в короткий путь
  без кириллицы, например `C:\CppProjects\parallel-data-processing-cpp`.
- **Permission denied при записи exe:** заверши предыдущий запуск через Shift+F5.
- **Долго работает / закончилась память:** начни с `--size 1000` для массивов
  или `--size 32` для матриц. Умножение матриц растёт по N^3.
- **Красные подчёркивания после clone:** сначала собери Debug, чтобы появился
  compile_commands.json, затем перезагрузи окно VS Code.
- **Нулевая ошибка:** допустима, особенно на небольших целых данных. Разный
  порядок сложения double может давать и ненулевую ошибку.

## Первичные инструкции

- [MSYS2: установка](https://www.msys2.org/)
- [MSYS2: среды и архитектуры](https://www.msys2.org/docs/environments/)
- [MSYS2: обновление](https://www.msys2.org/docs/updating/)
- [VS Code: GCC на Windows](https://code.visualstudio.com/docs/cpp/config-mingw)
- [CMake: FindOpenMP](https://cmake.org/cmake/help/latest/module/FindOpenMP.html)
- [Homebrew: libomp](https://formulae.brew.sh/formula/libomp)
