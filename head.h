
#ifndef LAB6_SOI_HEAD_H
#define LAB6_SOI_HEAD_H

#include <unistd.h>
#include <malloc.h>
#include <stdlib.h>
#include <memory.h>
#include <stdint.h>
#include <string.h>

#define ROZMIAR_BLOKU 1024 * 1024 //1 MB
#define LICZBA_BLOKOW 20
#define DLUGOSC_NAZWY 31
#define LICZBA_PLIKOW 10


int CzyJestTakNazwanyPlikNaWirtualnym(char *nazwaPliku, FILE *dysk, int dlugosc_nazwy);

uint JakiRozmiarPliku(FILE *file);

int czyJestMiejsce(uint rozmiarPliku, FILE *dysk, int *nrPierwszegoBloku, int *liczbaBlokow);

typedef uint32_t uint;

typedef struct SPlik {
    int status;
    char nazwa[DLUGOSC_NAZWY];
    uint rozmiar;//w bajtach
    uint ades_pierwszego_bloku;
    uint liczba_blokow;
} Plik;

typedef struct SBlok {
    int status;
    char dane[ROZMIAR_BLOKU];
} Blok;

typedef struct SSuperBlok {
    uint rozmiar_systemu; // w bajtach
    uint liczba_blokow;
    uint liczba_blokow_zajetych;
    Plik pliki[LICZBA_PLIKOW]; // first inode index
} SuperBlok;

void StworzWirtualnyDysk() {
    int i;
    FILE *dysk;
    SuperBlok superBlok;
    Blok blok;

    dysk = fopen("ToJestWirtualnyDysk", "w+b");

    if (!dysk) {
        printf("Blad, nie stworzono dysku\n");
        return;
    }
    uint rozmiarSystemu = (uint) sizeof(SuperBlok) + (uint) LICZBA_BLOKOW * (uint) sizeof(Blok);
    truncate("ToJestWirtualnyDysk", rozmiarSystemu);


    superBlok.liczba_blokow = LICZBA_BLOKOW;
    superBlok.liczba_blokow_zajetych = 0;

    for (i = 0; i < LICZBA_PLIKOW; i++) {
        superBlok.pliki[i].status = 0;
    }
    superBlok.rozmiar_systemu = rozmiarSystemu;
    rewind(dysk);
    if (fwrite(&superBlok, sizeof(superBlok), 1, dysk) != 1) {
        printf("Nie udało się poprawnie stowrzyc dysku\n");
        fclose(dysk);
        return;
    }

    blok.status = 0;
    for (i = 0; i < LICZBA_BLOKOW; i++) {
        if (fwrite(&blok, sizeof(blok), 1, dysk) != 1) {
            printf("Nie udało się poprawnie stowrzyc dysku\n");
            fclose(dysk);
            return;
        }
    }

    printf("Udało się poprawnie stowrzyc dysk\n");

    fclose(dysk);
    return;
}

void UsunWirtualnyDysk() {
    remove("ToJestWirtualnyDysk");
    printf("Dysk zostal skasowany\n");
}

void KopiujZFizycznegoNaWirtualny(char *nazwaPliku) {
    FILE *dysk, *plikNaFizycznym;
    uint dlugosc_nazwy;
    uint rozmiarPliku;
    int nrPierwszegoBloku;
    int liczbaBlokow;
    char *buffor;
    SuperBlok temp2;
    Blok temp3;
    int bufforZnacznik;
    int temp;
    int i;
    int statusBloku;
    char daneBloku;
    int jestWolnePlik;
    jestWolnePlik = 0;


    bufforZnacznik = 0;

    plikNaFizycznym = fopen(nazwaPliku, "rb");
    dysk = fopen("ToJestWirtualnyDysk", "r+b");

    if (!plikNaFizycznym) {
        printf("Blad, nie udalo sie otworzyc pliku na dysku fizycznym. Nazwa pliku: %s\n", nazwaPliku);
        return;
    }

    if (!dysk) {
        printf("Blad, nie udalo sie otworzyc pliku na dysku wirtualnym. \n");
        return;
    }

    dlugosc_nazwy = strlen(nazwaPliku);

    rewind(dysk);
    temp = fread(&temp2, sizeof(SuperBlok), 1, dysk);
    if (temp != 1) {
        fputs("Reading error !!!asd", stderr);
        return;
    }
    for (i = 0; i < LICZBA_PLIKOW; i++) {
        if (temp2.pliki[i].status == 0)
            jestWolnePlik = jestWolnePlik + 1;
    }
    if (jestWolnePlik == 0) {
        printf("Nie można dodac wiecej plikow. Osiagnieto limit liczby plikow\n");
        return;
    }

    if (dlugosc_nazwy >= DLUGOSC_NAZWY) {
        printf("Za długa nazwa\n");
        fclose(dysk);
        fclose(plikNaFizycznym);
        return;
    }

    if (CzyJestTakNazwanyPlikNaWirtualnym(nazwaPliku, dysk, dlugosc_nazwy) == 1) {
        printf("Blad, plik o takiej nazwie juz jest na wirtualnym dysku\n");
        fclose(dysk);
        fclose(plikNaFizycznym);
        return;
    }
    rozmiarPliku = JakiRozmiarPliku(plikNaFizycznym);

    if ((czyJestMiejsce(rozmiarPliku, dysk, &nrPierwszegoBloku, &liczbaBlokow)) == 0) {
        printf("Nie ma już miejsca na dysku wirtualnym\n");
        fclose(dysk);
        fclose(plikNaFizycznym);
        return;
    }


    fseek(dysk, sizeof(SuperBlok) + nrPierwszegoBloku * sizeof(Blok), 0);
    for (i = 0; i < liczbaBlokow; i++) {
        temp3.status = 1;
        if (i == liczbaBlokow - 1) {
            temp = fread(&(temp3.dane[0]), rozmiarPliku - (liczbaBlokow - 1) * ROZMIAR_BLOKU, 1, plikNaFizycznym);
            if (temp != 1) {
                fputs("Reading error !!!2", stderr);
                return;
            }
        } else {
            temp = fread(&(temp3.dane[0]), ROZMIAR_BLOKU, 1, plikNaFizycznym);
            if (temp != 1) {
                fputs("Reading error !!!3", stderr);
                return;
            }
        }
        temp = fwrite(&temp3, sizeof(Blok), 1, dysk);
        if (temp != 1) {
            fputs("Writing error !!!4", stderr);
            return;
        }

    }

    rewind(dysk);
    temp = fread(&temp2, sizeof(SuperBlok), 1, dysk);
    if (temp != 1) {
        fputs("Reading error !!!", stderr);
        return;
    }
    temp2.liczba_blokow_zajetych = temp2.liczba_blokow_zajetych + liczbaBlokow;
    for (i = 0; i < LICZBA_PLIKOW; i++) {
        if (temp2.pliki[i].status == 0) {
            strncpy(temp2.pliki[i].nazwa, nazwaPliku, dlugosc_nazwy);
            temp2.pliki[i].status = 1;
            temp2.pliki[i].liczba_blokow = liczbaBlokow;
            temp2.pliki[i].ades_pierwszego_bloku = nrPierwszegoBloku;
            temp2.pliki[i].rozmiar = rozmiarPliku;

            rewind(dysk);
            temp = fwrite(&temp2, sizeof(SuperBlok), 1, dysk);
            if (temp != 1) {
                fputs("Reading error !!!!!", stderr);
                return;
            }
            printf("Dodano plik: %s\n", nazwaPliku);
            return;
        }
    }

}

void PokazKatalog() {
    FILE *dysk;
    int temp;
    SuperBlok temp2;
    int i;
    int licznik;
    licznik = 0;
    dysk = fopen("ToJestWirtualnyDysk", "rb");
    rewind(dysk);
    temp = fread(&temp2, sizeof(SuperBlok), 1, dysk);
    if (temp != 1) {
        fputs("Reading error !!!", stderr);
        return;
    }
    for (i = 0; i < LICZBA_PLIKOW; i++) {
        if (temp2.pliki[i].status == 1) {
            printf("____________________________________\n");
            printf("nazwa:                 %s\n", temp2.pliki[i].nazwa);
            printf("rozmiar:               %d\n", temp2.pliki[i].rozmiar);
            printf("ades_pierwszego_bloku: %d\n", temp2.pliki[i].ades_pierwszego_bloku);
            printf("liczba_blokow:         %d\n", temp2.pliki[i].liczba_blokow);
            printf("____________________________________\n");
            licznik = licznik + 1;
        }
        if (i == LICZBA_PLIKOW - 1 && licznik == 0) {
            printf("Brak plików w katalogu\n");
        }
    }

}

void PokazMapeDysku() {
    FILE *dysk;
    SuperBlok temp2;
    Blok temp3;
    int temp;
    int i, j, k, bylo;
    dysk = fopen("ToJestWirtualnyDysk", "rb");
    temp = fread(&temp2, sizeof(SuperBlok), 1, dysk);
    if (temp != 1) {
        fputs("Reading error !!!", stderr);
        return;
    }

    printf("ADRES     |NUMER BLOKU |TYP OBSZARU |ROZMIAR    |STAN |NALEZY DO PLIKU                |\n");
    printf("---------------------------------------------------------------------------------------\n");
    printf("0         |-1          |SUPER BLOK  |%-11d|1    |NIE DOTYCZY                    |\n", sizeof(SuperBlok));
    for (i = 0; i < LICZBA_BLOKOW; i++) {
        bylo = 0;
        temp = fread(&temp3, sizeof(Blok), 1, dysk);
        if (temp != 1) {
            fputs("Reading error !!!", stderr);
            return;
        }
        for (j = 0; j < LICZBA_PLIKOW; j++) {
            if (temp2.pliki[j].status == 1) {
                for (k = temp2.pliki[j].ades_pierwszego_bloku;
                     k < temp2.pliki[j].ades_pierwszego_bloku + temp2.pliki[j].liczba_blokow; k++) {
                    if (k == i) {
                        printf("%-10d|%-12d|BLOK        |%-11d|%-5d|%-30s |\n", ftell(dysk) - sizeof(Blok), i,
                               sizeof(Blok),
                               temp3.status, temp2.pliki[j].nazwa);
                        bylo = 1;
                    }
                }
            }
        }
        if (bylo == 1) {
            continue;
        }
        printf("%-10d|%-12d|BLOK        |%-11d|%-5d|%-30s |\n", ftell(dysk) - sizeof(Blok), i, sizeof(Blok),
               temp3.status, "BRAK");
    }
}

void usunPlikZDysku(char *nazwaPliku) {
    FILE *dysk;
    SuperBlok temp2;
    Blok temp3;
    int temp;
    int i, j, k, bylo;
    bylo = -1;
    dysk = fopen("ToJestWirtualnyDysk", "r+b");
    temp = fread(&temp2, sizeof(SuperBlok), 1, dysk);
    if (temp != 1) {
        fputs("Reading error !!!", stderr);
        return;
    }
    for (i = 0; i < LICZBA_PLIKOW; i++) {
        if (temp2.pliki[i].status == 1) {
            if (strncmp(temp2.pliki[i].nazwa, nazwaPliku, strlen(nazwaPliku)) == 0) {
                for (j = temp2.pliki[i].ades_pierwszego_bloku;
                     j < temp2.pliki[i].ades_pierwszego_bloku + temp2.pliki[i].liczba_blokow; j++) {
                    fseek(dysk, sizeof(SuperBlok) + j * sizeof(Blok), 0);
                    temp3.status = 0;
                    temp = fwrite(&temp3, sizeof(Blok), 1, dysk);
                    if (temp != 1) {
                        fputs("Writing error !!!", stderr);
                        return;
                    }
                }
                temp2.pliki[i].status = 0;
                rewind(dysk);
                temp = fwrite(&temp2, sizeof(SuperBlok), 1, dysk);
                if (temp != 1) {
                    fputs("Writing error !!!", stderr);
                    return;
                }
                printf("Usunięto plik: %s\n", nazwaPliku);
                return;
            }
        }
    }
}

void KopiujZWirtualnegoNaFizyczny(char *nazwaPliku) {
    FILE *dysk, *plikNaFizycznym;
    uint dlugosc_nazwy;
    uint rozmiarPliku;
    int nrPierwszegoBloku;
    int liczbaBlokow;
    SuperBlok temp2;
    Blok temp3;
    int bufforZnacznik;
    int temp;
    int i;
    int nrPliku;


    bufforZnacznik = 0;

    dysk = fopen("ToJestWirtualnyDysk", "rb");

    if (!dysk) {
        printf("Blad, nie udalo sie otworzyc pliku na dysku wirtualnym. \n");
        return;
    }

    temp = fread(&temp2, sizeof(SuperBlok), 1, dysk);
    if (temp != 1) {
        fputs("Reading error !!!", stderr);
        return;
    }

    dlugosc_nazwy = strlen(nazwaPliku);

    for (i = 0; i < LICZBA_PLIKOW; i++) {
        if (temp2.pliki[i].status == 1) {
            if (strncmp(temp2.pliki[i].nazwa, nazwaPliku, dlugosc_nazwy) == 0) {
                nrPierwszegoBloku = temp2.pliki[i].ades_pierwszego_bloku;
                liczbaBlokow = temp2.pliki[i].liczba_blokow;
                rozmiarPliku = temp2.pliki[i].rozmiar;
                nrPliku = i;
            }
        }
    }


    printf("%d %d\n", nrPierwszegoBloku, liczbaBlokow);///////////////////////////////

    plikNaFizycznym = fopen(nazwaPliku, "w+b");

    if (!plikNaFizycznym) {
        printf("Blad, nie udalo sie otworzyc pliku na dysku fizycznym. Nazwa pliku: %s\n", nazwaPliku);
        return;
    }

    rewind(plikNaFizycznym);
    fseek(dysk, sizeof(SuperBlok) + nrPierwszegoBloku * sizeof(Blok), 0);
    for (i = 0; i < liczbaBlokow; i++) {
        temp = fread(&temp3, sizeof(Blok), 1, dysk);
        if (temp != 1) {
            fputs("Reading error !!!", stderr);
            return;
        }
        if (i == liczbaBlokow - 1) {
            temp = fwrite(&temp3.dane[0], rozmiarPliku - (liczbaBlokow - 1) * ROZMIAR_BLOKU, 1, plikNaFizycznym);
            if (temp != 1) {
                fputs("Reading error !!!", stderr);
                return;
            }
        } else {
            temp = fwrite(&temp3.dane[0], ROZMIAR_BLOKU, 1, plikNaFizycznym);
            if (temp != 1) {
                fputs("Reading error !!!", stderr);
                return;
            }
        }
    }
}

int CzyJestTakNazwanyPlikNaWirtualnym(char *nazwaPliku, FILE *dysk, int dlugosc_nazwy) {
    int i;
    SuperBlok temp;
    rewind(dysk);
    fread(&temp, sizeof(SuperBlok), 1, dysk);
    for (i = 0; i < LICZBA_PLIKOW; i++) {
        if (temp.pliki[i].status == 1) {
            if (strncmp(temp.pliki[i].nazwa, nazwaPliku, dlugosc_nazwy) == 0) {
                rewind(dysk);
                return 1;
            }
        }
    }
    rewind(dysk);
    return 0;
}

uint JakiRozmiarPliku(FILE *file) {
    uint rozmiar;
    fseek(file, 0, SEEK_END);
    rozmiar = (uint) ftell(file);
    rewind(file);

    return rozmiar;
}

int czyJestMiejsce(uint rozmiarPliku, FILE *dysk, int *nrPierwszegoBloku, int *liczbaBlokow) {
    uint wolneMiejsce;
    int i;
    Blok temp3;
    int temp;
    wolneMiejsce = 0;
    *nrPierwszegoBloku = 0;
    *liczbaBlokow = 0;
    fseek(dysk, sizeof(SuperBlok), 0);
    for (i = 0; i < LICZBA_BLOKOW; i++) {
        temp = fread(&temp3, sizeof(Blok), 1, dysk);
        if (temp != 1) {
            fputs("Reading error !!!\n", stderr);
            exit(0);
        }
        if (temp3.status == 0 && wolneMiejsce == 0) {
            *nrPierwszegoBloku = i;
            *liczbaBlokow = *liczbaBlokow + 1;
            wolneMiejsce += ROZMIAR_BLOKU;
            if (rozmiarPliku <= wolneMiejsce) {
                return 1;
            }
        } else if (temp3.status == 0 && wolneMiejsce != 0) {
            wolneMiejsce += ROZMIAR_BLOKU;
            *liczbaBlokow = *liczbaBlokow + 1;
            if (rozmiarPliku <= wolneMiejsce) {
                return 1;
            }
        } else if (temp3.status == 1) {
            *nrPierwszegoBloku = 0;
            *liczbaBlokow = 0;
            wolneMiejsce = 0;
        }
    }
    printf("\n");
    return 0;
}


#endif //LAB6_SOI_HEAD_H
