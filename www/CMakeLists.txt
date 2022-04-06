
add_library(www INTERFACE index.html qmlprojects/)

FILE(COPY index.html DESTINATION "${CMAKE_BINARY_DIR}")

FILE(COPY uploadIcon.svg DESTINATION "${CMAKE_BINARY_DIR}")
FILE(COPY qtdesignstudioviewer-16.png DESTINATION "${CMAKE_BINARY_DIR}")
FILE(COPY qtdesignstudioviewer-32.png DESTINATION "${CMAKE_BINARY_DIR}")
FILE(COPY qtdesignstudioviewer-48.png DESTINATION "${CMAKE_BINARY_DIR}")
FILE(COPY qtdesignstudioviewer-128.png DESTINATION "${CMAKE_BINARY_DIR}")
FILE(COPY qtdesignstudioviewer-256.png DESTINATION "${CMAKE_BINARY_DIR}")
FILE(COPY qtdesignstudioviewer-384.png DESTINATION "${CMAKE_BINARY_DIR}")
FILE(COPY qtdesignstudioviewer-512.png DESTINATION "${CMAKE_BINARY_DIR}")

FILE(COPY Titillium-Light.otf DESTINATION "${CMAKE_BINARY_DIR}")
FILE(COPY Titillium-ThinItalic.otf DESTINATION "${CMAKE_BINARY_DIR}")
FILE(COPY Titillium-Thin.otf DESTINATION "${CMAKE_BINARY_DIR}")

FILE(COPY qmlprojects/index.json DESTINATION "${CMAKE_BINARY_DIR}/qmlprojects/index.json")

