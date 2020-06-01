
#include "head.h"

int main(int argc, char* argv[]) {
    if (argc < 2) {
        printf("Brak argumentow\n");
        return -1;
    }
    switch (argv[1][1]) {
        case 'c' ://create
            StworzWirtualnyDysk();
            break;
        case 'r' ://remove
            UsunWirtualnyDysk();
            break;
        case 's' ://save on virtual
            if (argc != 3) {
                printf("Za malo argumrntow\n");
                return -1;
            }
            KopiujZFizycznegoNaWirtualny(argv[2]);
            break;
        case 'a' ://show all files
            PokazKatalog();
            break;
        case 'm' ://disk map
            PokazMapeDysku();
            break;
        case 'd' ://delete file
            if (argc != 3) {
                printf("Za malo argumrntow\n");
                return -1;
            }
            usunPlikZDysku(argv[2]);
            break;
        case 'p' ://save on physical
            if (argc != 3) {
                printf("Za malo argumrntow\n");
                return -1;
            }
            KopiujZWirtualnegoNaFizyczny(argv[2]);
            break;
    }
}
