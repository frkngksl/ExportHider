# ExportHider
ExportHider generates a C++ DLL template which contains a code stub that allows you to hide Exported Functions from the Export Directory of the DLL on the filesystem. After putting the function definitions and compiling the file, you won't see the hidden export functions through PE File Viewers like PEBear or CFF Explorer. However, since the code stub in the template recreates the Export Directory during runtime, legitimate GetProcAddress calls would still work. 

# How It Works?
Normally, when you want to define an Exported Function in the DLL files (in C or C++), you simply put a **`__declspec(dllexport)`** keyword before the function name, or create a .def file. After the compilation, the compiler creates a specific table called Export Directory for holding the information related to Exported Functions. The structure of the Export Directory can be seen below:

<p align="center">
  <img src="https://github.com/frkngksl/ExportHider/assets/26549173/a1a80e2b-7946-46a5-bba8-be33cc760d88">
</p>

When a process wants to use a function from a DLL file, Windows Loader simply parses this structure and imports the requested functions by using the `AddressOfFunctions`, `AddressOfNames`, `AddressOfNameOrdinals` arrays. 

The specific routine for importing a function is explained by [ferreirasc's blog post](https://ferreirasc.github.io/PE-Export-Address-Table/) in detail, but shortly for a function imported by name, the Loader iterates the `AddressOfNames` array (values of this array are just RVA values) and searches the given name. Once, the loader has a match in the “i” position, it will refer to the ith index of `AddressOfNameOrdinals` array and get the ordinal associated with this function. Having the ordinal, the loader will refer to AddressOfFunctions on the ordinal value position to finally get the RVA associated with the imported function.

<p align="center">
  <img src="https://github.com/frkngksl/ExportHider/assets/26549173/59d616e4-f00d-4e07-8f17-2f319097945e">
</p>

The crucial point here is that all these search and access operations by the Windows Loader are done after the DLL is mapped to the process address space. During DLL mapping (or calling LoadLibrary function), the whole DLL file including its PE Headers is written to the memory, and the Loader parses the headers in the memory to reach the Export Directory. This means that if the DLL itself is able to overwrite its PE headers in the memory to change the Export Directory Address (simply the 0th index of the `DataDirectory`) after attaching to the process, the loader can look for functions to import in an arbitrary Export Directory rather than in the Export Directory added by the compiler.


# References
- https://rioasmara.com/2021/10/10/analyze-dll-export-with-pe-bear/
- https://ferreirasc.github.io/PE-Export-Address-Table/
