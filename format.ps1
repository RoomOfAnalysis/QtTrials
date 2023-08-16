$files = (git ls-files --exclude-standard);

$extensions = ".cpp", ".h", ".hpp", ".c", ".cc", ".cxx", ".hxx", ".ixx"

foreach ($file in $files) {

    if ((Get-Item $file).Extension -in $extensions) {

        # Write-Output $file

        clang-format -i -style=file $file

    }

}