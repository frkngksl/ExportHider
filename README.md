![image](https://github.com/frkngksl/ExportHider/assets/26549173/4b325f62-864f-45a5-80e6-558f997179ae)
![image](https://github.com/frkngksl/ExportHider/assets/26549173/35db34fc-dded-4fd1-8468-10ab99de439b)
# ExportHider
ExportHider generates a C++ DLL template which contains a code stub that allows you to hide Exported Functions from the Export Directory of the DLL on the filesystem. After putting the function definitions and compiling the file, you won't see the hidden export functions through PE File Viewers like PEBear or CFF Explorer. However, since the code stub in the template recreates the Export Directory during runtime, legitimate GetProcAddress calls would still work. It might be useful for DLL Sideloading or DLL stagers. 

# How It Works?
Normally, when you want to define an Exported Function in the DLL files (in C or C++), you simply put a **`__declspec(dllexport)`** keyword before the function name, or create a .def file. After the compilation, the compiler creates a specific table called Export Directory for holding the information related to Exported Functions. The structure of the Export Directory can be seen below:

<p align="center">
  <img src="https://github.com/frkngksl/ExportHider/assets/26549173/a1a80e2b-7946-46a5-bba8-be33cc760d88">
</p>

When a process wants to use a function from a DLL file, Windows Loader simply parses this structure and imports the requested functions by using the `AddressOfFunctions`, `AddressOfNames`, `AddressOfNameOrdinals` arrays. 

The specific routine for importing a function is explained by [ferreirasc's blog post](https://ferreirasc.github.io/PE-Export-Address-Table/) in detail, but shortly for a function imported by name, the Loader iterates the `AddressOfNames` array (values of this array are just RVA values) and searches the given name. Once, the loader has a match in the “i” position, it will refer to the ith index of `AddressOfNameOrdinals` array and get the ordinal associated with this function. Having the ordinal, the loader will refer to `AddressOfFunctions` on the ordinal value position to finally get the RVA associated with the imported function.

<p align="center">
  <img src="https://github.com/frkngksl/ExportHider/assets/26549173/59d616e4-f00d-4e07-8f17-2f319097945e">
</p>

The crucial point here is that all these search and access operations by the Windows Loader are done after the DLL is mapped to the process address space. During DLL mapping (or calling LoadLibrary function), the whole DLL file including its PE Headers is written to the memory, and the Loader parses the headers in the memory to reach the Export Directory. This means that if the DLL itself is able to overwrite its PE headers in the memory to change the Export Directory Address (simply the 0th index of the `DataDirectory`) after attaching to the process, the loader can look for functions to import in an arbitrary Export Directory rather than in the Export Directory added by the compiler.

<p align="center">
  <img src="https://github.com/frkngksl/ExportHider/assets/26549173/0c61aebe-5bc1-4e58-939a-bfdb0329c875">
</p>

# Command Line Parameters

```

   __                       _          _     _
  /__\_  ___ __   ___  _ __| |_  /\  /(_) __| | ___ _ __
 /_\ \ \/ / '_ \ / _ \| '__| __|/ /_/ / |/ _` |/ _ \ '__|
//__  >  <| |_) | (_) | |  | |_/ __  /| | (_| |  __/ |
\__/ /_/\_\ .__/ \___/|_|   \__\/ /_/ |_|\__,_|\___|_|
          |_|
                     by @R0h1rr1m

Usage of C:\Users\Public\DLLDemo\ExportHider.exe:

    -h | --help                                 Show the help message.
    -i | --input <Input Path>                   Input path for the list of function names to be hidden. (Mandatory)
    -o | --output <Output Path>                 Output path for the DLL template. (Mandatory)
    -n | --name <DLL Name>                      Name of the DLL for the Export Directory. (Mandatory)
    -c | --count <Number of Other Functions>    Number of other exported functions that won't be hidden.
```
Regarding the `-i | --input <Input Path>` parameter, you need to specify a path for the input file that stores the function names to be hidden line by line. The content of an example input file would be: 
```
TestFunction1
TestFunction2
TestFunction3
```

Regarding the `-c | --count` parameter, if you don't want to hide all your exported functions (i.e. there are some functions exported with `__declspec(dllexport)` or .def file, and they appear in the Export Directory of the DLL file in the filesystem via PE File Viewers), just specify how many there are by using this parameter because the tool needs this information when making memory calculations. 


# Quick Demo Video

[<img src="https://github.com/frkngksl/ExportHider/assets/26549173/bf6a3513-2394-4cbd-8cd5-a77cab44d43f" width="100%">](https://www.youtube.com/watch?v=ylYd89nvLEk "ExportHider Quick Demo")        


# Workarounds

If you want to play with the technique, there are two interesting points that I encountered during the development of the project. You might need to know them before changing the project:

- Windows Loader uses a Binary Search-like algorithm to find the exported function if you call the `GetProcAddress` function with a name. Because of that, the names of all exported functions including the hidden ones need to be sorted in the `AddressOfNames` array. Otherwise, the `GetProcAddress` function returns NULL. Therefore, I used the Bubble Sort algorithm to sort the members of this array.
- As I said above, the `AddressOfNames` array, `AddressOfFunctions` array, `DataDirectory` array, and some other fields require values in Relative Virtual Addresses (RVAs). In addition to that, they hold these RVA values in DWORD-sized fields. When you allocate a memory region for a new arbitrary export directory by using Dynamic Memory Allocation Functions like `VirtualAlloc` or `HeapAlloc`, given addresses will be far from the DLL mapped region and RVA values don't fit into the DWORD-sized fields and you encounter with integer overflow. That's why I used global variables with the byte array type in the DLL template for memory requirements.

# References
- https://rioasmara.com/2021/10/10/analyze-dll-export-with-pe-bear/
- https://ferreirasc.github.io/PE-Export-Address-Table/
