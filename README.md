# forte

Учебный макет среды исполнения IEC 61499 (аналог FORTE).  
Сервер принимает XML команды от 4diac по TCP, загружает конфигурации из `.fboot` и позволяет проводить тестирование и фаззинг.

---

## Структура проекта

```text
FORTE/
    build/                              # Каталог для сборки (создаётся вручную или CMake)
    pugixml/                            # Встроенный исходный код pugixml (из архива releases)
    CMakeLists.txt                      # Конфигурация CMake
    ПМИ.md                              # Программа и методика испытаний
    Technical specification.md          # ТЗ
    use-case_tests.png                  # Диаграмма тестирования
    command_handler.h
    command_handler.cpp
    server.h
    server.cpp
    main.cpp
    test_client.cpp
    code_analyzer.cpp
    README.md

## Сборка

1. Установка pugixml

    1.1. Перейти на страницу релизов pugixml: https://github.com/zeux/pugixml/releases
    1.2. Скачать архив pugixml-1.15.zip

2. Распаковать в проект

    2.1. Создать в корне репозитория папку pugixml/
    2.2. Распаковать архив так, чтобы структура была примерно такой:

    forte/
        pugixml/
            src/
                pugixml.hpp
                pugixml.cpp
                pugiconfig.hpp
            CMakeLists.txt
            …
3. Сборка проекта

# 1. Создать каталог сборки
mkdir build
cd build

# 2. Сгенерировать проект
cmake ..

# 3. Собрать
cmake --build . --config Release

Файл будет находиться по адресу: build\Release\forte.exe

# 4. Запуск
build\Release\forte.exe ..\test.fboot