# Инструкции агенту: развёртывание учебного проекта

## Назначение

Это практические по параллельной обработке данных: C++20, OpenMP, VS Code.
Общайся с владельцем по-русски. Алгоритмы должны оставаться понятными студенту:
обычные функции, циклы, массивы и директивы OpenMP. Инфраструктура находится
в include/, scripts/, tests/ и не должна усложнять учебные функции.
Подробная инструкция установки — README.md.

## Сначала осмотри среду

1. Прочитай README.md и проверь ОС/архитектуру, git status, наличие компилятора,
   CMake, Ninja и OpenMP. Убедись, что git rev-parse --show-toplevel указывает
   именно на этот репозиторий, а не на родительскую папку пользователя.
2. На Windows используй путь без кириллицы, MSYS2 UCRT64 GCC, не MSVC и не
   MSYS /usr/bin/g++. На ARM64 Windows предпочти WSL ARM64; этот Windows-набор
   настроен на x86_64. На Linux используй GCC, на macOS Homebrew LLVM + libomp.
3. Исходники разных практик содержат отдельные main(): нельзя собирать g++ *.cpp.
4. Не ищи токены в файлах пользователя. Приватный репозиторий требует доступа
   авторизованного аккаунта; git/gh уже могут быть настроены на новом компьютере.

## Windows: выполнить после clone

Если MSYS2 отсутствует, установи его официальным установщиком/winget по README.
Обнови пакеты через MSYS2 UCRT64 pacman -Syu; обновление runtime иногда требует
нового процесса терминала и повторного запуска команды.

```powershell
powershell -NoProfile -ExecutionPolicy Bypass -File scripts/setup-windows.ps1
powershell -NoProfile -ExecutionPolicy Bypass -File scripts/build.ps1 -Test
powershell -NoProfile -ExecutionPolicy Bypass -File scripts/run.ps1 -Practice 1 -Size 1000 -NoBuild
powershell -NoProfile -ExecutionPolicy Bypass -File scripts/run.ps1 -Practice 2 -Size 1000 -NoBuild
powershell -NoProfile -ExecutionPolicy Bypass -File scripts/run.ps1 -Practice 3 -Task 2 -Size 32 -Repeats 2 -NoBuild
```

Другой путь MSYS2: -MsysRoot у setup, CPP_TOOLCHAIN_BIN или -ToolchainBin у build/run.
Для F5 настрой course.toolchainBin в .vscode/settings.json. Пути debugger и
сборки используют эту настройку; compile_commands.json создаётся автоматически.
Установи ms-vscode.cpptools; конфликтующее расширение C/C++ Compile Run может
перехватить F5. Подробности и исправление привязки есть в README.

## Linux / macOS

Linux: установи g++, gdb, cmake, ninja-build. macOS: brew install cmake ninja llvm libomp.
Затем:

```bash
bash scripts/build.sh debug all
ctest --test-dir build/debug --output-on-failure
OMP_NUM_THREADS=2 ./build/debug/practice01 --size 1000
OMP_NUM_THREADS=2 ./build/debug/practice02 --size 1000
OMP_NUM_THREADS=2 ./build/debug/practice03 --task 2 --size 32 --repeats 2
```

build.sh определяет Homebrew prefix на macOS. Для нативного F5 на macOS установи
vadimcn.vscode-lldb и выбери конфигурацию с суффиксом (macOS, LLDB).
Если используешь свою CMake-команду на macOS, задай Homebrew clang++ и prefix libomp.

## Что считать успешным развёртыванием

- CMake нашёл OpenMP, собраны practice01, practice02, practice03 и course_checks.
- Все CTest-проверки прошли, все три маленьких запуска завершились с кодом 0.
- Для третьей практики проверь также --task 0, 1 и 3: CTest уже делает это.
- На Windows проверь GDB на executable из build/debug; для полной проверки UI
  выбери практику в Run and Debug и запусти F5, если доступно управление VS Code.
  CLI/GDB-проверку не выдавай за реальное нажатие F5.
- Сообщи, что установлено, какие команды выполнены и остались ли ограничения.

## Изменения и проверки

- Общая сборка — CMakeLists.txt, OpenMP через OpenMP::OpenMP_CXX. Не теряй -fopenmp.
- Debug — для отладки, Release — для измерения с -O2 без fast-math.
- Первые два main() без аргументов сохраняют большой массив 2 ГиБ.
  Для smoke tests всегда используй --size 1000 или 1048576. Не запускай
  большой эксперимент только ради проверки установки.
- У practice03 --size означает сторону квадратной матрицы, не количество элементов.
  Не используй размер 268435456 для матрицы. Умножение имеет сложность N^3.
- Учебное значение task в main() можно менять; аргумент --task переопределяет его.
- Сохраняй короткий вывод: сумма/результат, ошибка и время. Не печатай всю большую матрицу.
- Не добивайся искусственно ненулевой ошибки. Нулевые ошибки на целых данных допустимы.
- Проверяй все элементы матрицы; совпадения одной общей суммы недостаточно.
- Храни main.cpp, practice02.cpp и practice03.cpp отдельно, без общего меню курса.
  Третье занятие пока подготовительное: не выдавай догадки за задание преподавателя.
- Оригиналы одногруппника в materials/practice02/original/ не редактируй.
  .exe и ZIP из исходного архива не запускай и не публикуй; собирай проверенные исходники.
- Запускай CTest после изменений алгоритмов, CLI или сборки. В tests/checks.cpp
  исходники подключены с переименованием main, чтобы проверить те же функции
  без переписывания учебных файлов в библиотеку.
- Генерируемые build/, output/, exe, obj и архивы игнорируются Git. Не коммить
  локальные пути пользователя, секреты, бинарники или CMake cache.
- Если меняешь компилятор, используй отдельную build-папку. Не удаляй каталоги
  пользователя и не перезаписывай его незакоммиченные изменения.
- Новый урок: отдельный practice04.cpp, цель CMake, задача и конфигурация VS Code,
  допустимая цель скриптов, маленький тест и запись в README.

## Автопроверка

.github/workflows/build.yml собирает и запускает CTest на Windows/MSYS2,
Ubuntu/GCC и macOS/Homebrew LLVM. Не называй платформу проверенной, пока её
job не завершился успешно. Сборка CI не проверяет UI VS Code или производительность.
