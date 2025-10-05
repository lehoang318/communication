$clangFormatPath = "clang-format.exe"

Start-Process -FilePath $clangFormatPath -ArgumentList "--verbose", "-i", "include/*.hpp" -Wait -NoNewWindow
Start-Process -FilePath $clangFormatPath -ArgumentList "--verbose", "-i", "include/inline/*.inl" -Wait -NoNewWindow
Start-Process -FilePath $clangFormatPath -ArgumentList "--verbose", "-i", "src/*.cpp" -Wait -NoNewWindow
