# PS5 build licenses

Commit this directory with the PS5 source changes. The repository root
`COPYING` covers aria2 itself; these files cover components used in the
PS5 ELF and embedded AriaNg page. Include the relevant notices when
redistributing the ELF. `make -f Makefile.ps5` refreshes these files from
the pinned or selected upstream sources. `make -f Makefile.ps5 clean` leaves
this directory intact.
