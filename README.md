# ExportHider
ExportHider generates a C++ DLL template which contains a code stub that allows you to hide Exported Functions from the Export Directory of the DLL on the filesystem. After putting the function definitions and compiling the file, you won't see the hidden export functions through PE File Viewers like CFF Explorer. However, since the code stub in the template recreates the Export Directory during runtime, legitimate GetProcAddress calls would be executed successfully. This method only works for Dynamic DLL loading or custom DLL loader cases.

# How It Works?
Normally, when you want to define an Exported Function in the DLL files (in C or C++), you simply put a **`__declspec(dllexport)`** keyword before the function name, or create a .def file. After the compilation, the compiler creates a specific table called the Export Directory for holding the information related to Exported Functions. The structure of the Export Directory can be seen below:

<p align="center">
  <img src="https://github.com/frkngksl/ExportHider/assets/26549173/a1a80e2b-7946-46a5-bba8-be33cc760d88">
</p>

When a process wants to use a function from a DLL file, Windows Loader simply parses this structure and imports the requested functions by using the `AddressOfFunctions`, `AddressOfNames`, `AddressOfNameOrdinals` arrays. 

The specific routine for importing a function is explained by [ferreirasc's blog post](https://ferreirasc.github.io/PE-Export-Address-Table/) in detail, but shortly, for a function imported by name, the Loader iterates the `AddressOfNames` array (values of this array are just RVA values) and searches the given name. Once the loader has a match in the “i” position, it will refer to the ith index of `AddressOfNameOrdinals` array and get the ordinal associated with this function. Having the ordinal, the loader will refer to `AddressOfFunctions` on the ordinal value position to finally get the RVA associated with the imported function.

<p align="center">
  <img src="https://github.com/frkngksl/ExportHider/assets/26549173/59d616e4-f00d-4e07-8f17-2f319097945e">
</p>

The crucial point here is that all these search and access operations by the Windows Loader, when LoadLibrary is called, are done after the DLL is mapped to the process address space. During DLL mapping, the whole DLL file, including its PE headers, is written to the memory, and the Loader parses the headers in the memory to reach the Export Directory. This means that if the DLL itself is able to overwrite its PE headers in the memory to change the Export Directory Address (simply the 0th index of the `DataDirectory`) after attaching to the process, the loader can look for functions to import in an arbitrary Export Directory rather than in the Export Directory added by the compiler.

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

- Windows Loader uses a Binary Search-like algorithm to find the exported function if you call the `GetProcAddress` function with a name. Because of that, the names of all exported functions, including the hidden ones, need to be sorted in the `AddressOfNames` array. Otherwise, the `GetProcAddress` function returns NULL. Therefore, I used the Bubble Sort algorithm to sort the members of this array.
- As I said above, the `AddressOfNames` array, `AddressOfFunctions` array, `DataDirectory` array, and some other fields require values in Relative Virtual Addresses (RVAs). In addition to that, they hold these RVA values in DWORD-sized fields. When you allocate a memory region for a new arbitrary export directory by using Dynamic Memory Allocation Functions like `VirtualAlloc` or `HeapAlloc`, given addresses will be far from the DLL mapped region, and RVA values don't fit into the DWORD-sized fields, and you encounter integer overflow. That's why I used global variables with the byte array type in the DLL template for memory requirements.

# Statically Imported DLL (a.k.a. DLL Sideloading) Case 

My first goal while creating this project was to generate a DLL that has missing exports (or a complete lack of an export table) but is still successfully loaded by the newly created process. I thought that behavior might bring a new playground for the DLL sideloading payloads. However, I couldn't find any function or a way that the export directory fixer stub runs before the Windows Loader checks for the exported DLL functions.

<img width="1360" height="580" alt="dll_timing_problem (1)" src="https://github.com/user-attachments/assets/5ffc6701-bf8d-46ca-9cdf-ccccbf2fab47" />

More technically, I noticed the following flow and function call for each DLL in the code sections related to the Windows Loader in NTDLL (gathered from my old notes; there might be some mistakes because I'm not an expert reverse engineer):

```
1. LdrpMapDll - This is where the DLL is mapped to the Process Address Space. All DLLs that satisfy the DLL name condition 
                are directly put into the memory, no precheck control for the filesystem version.
2. LdrpSnapModule - This is where the Windows Loader starts to resolve imports. For each import descriptor, it parses the PE 
                    structure, checks the export table, binary searching for the imported function, calculating RVA of that,
                    and writes its address to the corresponding caller's process' Import Address Table entry during this function.
3. LdrpDoPostSnapWork - If step 2 succeeds for each imported function, memory protections, TLS initialization, CFG enablement
                        are done in this function.
4. LdrpInitializeNode - If step 3 succeeds, there are module linking functions in this step.
5. LdrpCallTlsInitializers - This is where the TLS callbacks are called before the DllMain function.
6. LdrpCallInitRoutine - This is where the DLLMain itself is called for the first time for the imported DLL. In the original 
                         solution, this function is too late to fix the export table.
```
When you run an executable that imports a DLL, if the Windows loader couldn't find the required function name in the export table of the DLL, it halts the execution, and it doesn't execute functions which are executed after ```LdrpSnapModule```.

For the DLL sideloading case, we cannot modify the caller process; thus, the only chance to fix the Export Table dynamically is by finding a code execution opportunity between ```LdrpMapDll``` and ```LdrpSnapModule``` functions because the Loader halts immediately during ```LdrpSnapModule``` function checks. I've tried TLS callbacks, second DLL loads, forwarded exports, and some other workarounds, but none of them helped me to find such a place, so unfortunately, this method doesn't work directly for DLL sideloading or statically imported DLLs. If you discover a solution or a viable workaround for this issue, I’d be very happy to explore it further — whether that means discussing the idea, thinking through the approach together, or implementing it. Any contribution in this direction would be greatly appreciated.

One possible way to use this method for DLL Sideloading cases is to split the whole work into two DLLs, namely the Proxy DLL and the Payload DLL. Proxy DLL takes the name the EXE expects, and it has visible exports that satisfy the loader's static import check, while Payload DLL contains the real hidden functionality, has missing exports, and reconstructs its export table in DllMain. I don't think this is a good workaround for this problem, so I didn't implement it.

# References
- https://rioasmara.com/2021/10/10/analyze-dll-export-with-pe-bear/
- https://ferreirasc.github.io/PE-Export-Address-Table/

# Disclaimer

For authorized security testing only. Misuse of this tool against systems without explicit permission is illegal.
