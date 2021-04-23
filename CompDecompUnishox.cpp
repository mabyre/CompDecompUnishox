/*--------------------------------------------------------------------------*\
 * DecompressUnishox.cpp
 *--------------------------------------------------------------------------
 * Copyright (c) 2021 SoDevLog. Written by BRy.
 *--------------------------------------------------------------------------
 * Creation   : 23/04/2021
 * Evolutions : None
 * Change log : 
\*--------------------------------------------------------------------------*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <conio.h>   /* _getche */
#include <io.h>      /* _finddata_t _findfirst ,_findnext, _findclose */
#include <unishox2.h>

/*--------------------------------------------------------------------------*/

#define MAXCAR    256    /* Max de caracteres dans un nom de fichier */
#define MAXLISTE  100    /* Max de fichiers a traiter dans un liste  */
#define MAXCHAINE 128    /* Longueur max des chaines                 */

#define MAX_BUFFER_SIZE_UNISHOX 1024

/*--------------------------------------------------------------------------*/

/* Debug functions */
#ifdef NO_DEBUG
#   include "pmTrace.h"
#   define  as_printf      pm_trace0
#else
#   define  as_printf	   printf
#   define  MainProgram    main
#endif

/*--------------------------------------------------------------------------*/

void DemandeParametres(void);
bool DecompressUnishox(FILE* fileIn, FILE* fileOut);
bool CompressUnishox(FILE* fileIn, FILE* fileOut);

/*--------------------------------------------------------------------------*/

void DemandeParametres(void)
{
    as_printf("\n");
    as_printf("Supra Couche DOS - BRy - Version 3.0\n\n");
    as_printf("Traiter les CR (LF) des fichiers Windows.\n\n");
    as_printf("Syntax:\n\n>dos2unix [-cndu]*.txt *.c *.* fichier.ext ...\n\n");
    as_printf("\t-d : demande de confirmation pour le traitement de chaque fichier\n");
    as_printf("\t-c : Compresser a la facon unishox\n");
    as_printf("\t-d : Decompress a la facon unishox\n");
    exit(EXIT_SUCCESS);

}/* fin de DemandeParametres() */

/*--------------------------------------------------------------------------*/

char liste_files[MAXLISTE][MAXCAR];
char liste_fichiers[MAXLISTE][MAXCAR];

/*--------------------------------------------------------------------------*/

int MainProgram(int argc, char* argv[])
{
    FILE* file_in, * file_out1 = NULL;
    struct      _finddata_t file_infos;
    long        handle_file;
    bool        isOk = false;
    int         nb_crlf = 0;
    int         nb_lf = 0;
    
    bool     demande_confirmation = false;
    bool     compress_unishox = false;
    bool     decompress_unishox = false;
    bool     sauvegarder_fichier = false;
    int      i, j;

    /* Initialisations */
    for (i = 0; i < MAXLISTE; i++)
    {
        liste_files[i][0] = '\x0';
        liste_fichiers[i][0] = '\x0';
    }

    /* Traiter les parametres */
    if (argc < 2)
    {
        DemandeParametres();
    }
    for (i = 1, j = 0; i < argc; i++) /* argv[0] : nom du programme en cours */
    {                                    /* argc    : compteur des arguments    */
        if (_strcmpi(argv[i], "?") == 0)
        {
            DemandeParametres();
        }
        else if (strncmp(argv[i], "-o", 2) == 0)
        {
            demande_confirmation = true;
        }
        else if (strncmp(argv[i], "-c", 2) == 0)
        {
            compress_unishox = true;
        }
        else if (strncmp(argv[i], "-d", 2) == 0)
        {
            decompress_unishox = true;
        }
        else
        {
            /* Si ce n'est pas une option, c'est un fichier */
            strcpy_s(liste_files[j], argv[i]);
            j = j + 1;
        }
    }/* fin du for ( argc ) */

    /* Constituer la liste des fichiers a filtrer */
    if (liste_files[0][0] != '\x0') /* la liste n'est pas vide */
    {
        as_printf("Liste des fichiers a traiter :\n");
    }
    i = 0;
    j = 0;
    while (liste_files[i][0] != '\x0') /* fin de la liste de fichiers */
    {
        handle_file = _findfirst(liste_files[i], &file_infos);
        if (handle_file == -1L)
        {
            as_printf("Pas de fichier %s dans le repertoire courant !\n", liste_files[i]);
            break;
        }
        else
        {
            do
            {
                /* Les fichiers . et .. ne sont pas a traiter */
                if (   (_strcmpi(file_infos.name, ".") != 0) 
                    && (strncmp(file_infos.name, "..", 2) != 0)
                   )
                {
                    as_printf("  %s\n", file_infos.name);
                    strcpy_s(liste_fichiers[j++], file_infos.name);
                }
            } 
            while (_findnext(handle_file, &file_infos) == 0);

            _findclose(handle_file);
            i++;
        }
    }
    if (j == 0)
    {
        as_printf("Pas de fichiers trouves.\n");
        exit(EXIT_SUCCESS);
    }

    /* Demander confirmation a l'utilisateur pour traiter les fichiers */
    if (demande_confirmation)
    {
        printf("Voulez vous continuer <o/n> ? : ");
        if (_getche() == 'n')
        {
            exit(EXIT_SUCCESS);
        }
        printf("\n");
    }

    /*------------------------------------------------------*\ 
     * Effectuer le traitement des fichiers dans la liste 
    \*------------------------------------------------------*/
    i = 0;
    errno_t err0;
    while (liste_fichiers[i][0] != '\x0') /* fin de la liste */
    {
        as_printf("Fichier %s : ", liste_fichiers[i]);

        /* Demander confirmation pour traiter ce fichier */
        if (demande_confirmation)
        {
            printf("Voulez vous continuer <o/n> ? : ");
            if (_getche() == 'n')
            {
                exit(EXIT_SUCCESS);
            }
            printf("\n");
        }

        /* Ouvrir le fichier a traiter */
        err0 = fopen_s(&file_in, liste_fichiers[i], "rb");
        if (file_in == NULL)
        {
            as_printf("Impossible d'ouvrir le fichier %s\n", liste_fichiers[i]);
            exit(EXIT_FAILURE);
        }

        /* Ouvrir le fichier temporaire 1 */
        err0 = fopen_s(&file_out1, "temp1", "wb");
        if (file_out1 == NULL)
        {
            as_printf("Impossible d'ouvrir le fichier temporaire 1 !\n");
            exit(EXIT_FAILURE);
        }

        /* Effectuer le traitement */
        if (decompress_unishox)
        {
            sauvegarder_fichier = DecompressUnishox(file_in, file_out1);
        }

        if (compress_unishox)
        {
            sauvegarder_fichier = CompressUnishox(file_in, file_out1);
        }

        /* Fermer les fichiers avant remove() et rename() */
        fclose(file_out1);
        fclose(file_in);


        /* Si le fichier est a sauvegarder */
        //sauvegarder_fichier = nb_crlf != 0 || nb_lf != 0;
        if (sauvegarder_fichier)
        {
            as_printf("Ok\n");

            if (decompress_unishox )
            {
                strcat_s(liste_fichiers[i], "_decomp");
            }

            if (compress_unishox)
            {
                strcat_s(liste_fichiers[i], "_comp");
            }

            /* Rennomer le fichier temporaire1 */
            if (rename("temp1", liste_fichiers[i]) != 0)
            {
                as_printf("Renommage du fichier temporaire1 impossible !\n");
                exit(EXIT_FAILURE);
            }

            /* Afficher les resultats */
            if (nb_crlf != 0)
            {
                as_printf("%3d CR/LF\n", nb_crlf);
            }
            if (nb_lf != 0)
            {
                as_printf("%3d LF\n", nb_lf);
            }
        }

        /* Passer au fichier suivant */
        i = i + 1;
    }

    return EXIT_SUCCESS;

}/* fin du main() */

/*--------------------------------------------------------------------------*\
 * Decompresser a la facon unishox
\*--------------------------------------------------------------------------*/
bool DecompressUnishox(FILE* fileIn, FILE* fileOut)
{
    int c;

    char* pReadBuffer;
    char* pDecompressBuffer;
    int compLen;
    int iChar;

    pReadBuffer = (char*)malloc(MAX_BUFFER_SIZE_UNISHOX);
    pDecompressBuffer = (char*)malloc(MAX_BUFFER_SIZE_UNISHOX);

    if (pReadBuffer == NULL )
        return false;

    if (pDecompressBuffer == NULL)
        return false;

    iChar = 0;
    while ((c = getc(fileIn)) != EOF)
    {
        pReadBuffer[iChar++] = c;

        if ( iChar >= 512 -1 )
        { 
            compLen = unishox2_decompress_simple(pReadBuffer, iChar, pDecompressBuffer);
            
            int iCharOut = 0;
            while (compLen >= 0)
            {
                putc(pDecompressBuffer[iCharOut++], fileOut);
                compLen -= 1;
            }
            
            iChar = 0;
        }
    }

    if (iChar > 0)
    {
        compLen = unishox2_decompress_simple(pReadBuffer, iChar, pDecompressBuffer);

        int iCharOut = 0;
        while (compLen > 0)
        {
            putc(pDecompressBuffer[iCharOut++], fileOut);
            compLen -= 1;
        }
    }

    free(pReadBuffer);
    free(pDecompressBuffer);

    return true;
}

/*--------------------------------------------------------------------------*\
 * Compresser a la facon unishox
\*--------------------------------------------------------------------------*/
bool CompressUnishox(FILE* fileIn, FILE* fileOut)
{
    int c;

    char* pReadBuffer;
    char* pCompressBuffer;
    int compLen;
    int iChar;

    pReadBuffer = (char*)malloc(MAX_BUFFER_SIZE_UNISHOX);
    pCompressBuffer = (char*)malloc(MAX_BUFFER_SIZE_UNISHOX);

    if (pReadBuffer == NULL)
        return false;

    if (pCompressBuffer == NULL)
        return false;

    iChar = 0;
    while ((c = getc(fileIn)) != EOF)
    {
        pReadBuffer[iChar++] = c;

        if (iChar >= 512 - 1)
        {
            compLen = unishox2_compress_simple(pReadBuffer, iChar, pCompressBuffer);

            int iCharOut = 0;
            while (compLen > 0)
            {
                putc(pCompressBuffer[iCharOut++], fileOut);
                compLen -= 1;
            }

            iChar = 0;
        }
    }

    if (iChar > 0)
    {
        compLen = unishox2_compress_simple(pReadBuffer, iChar, pCompressBuffer);

        int iCharOut = 0;
        while (compLen >= 0)
        {
            putc(pCompressBuffer[iCharOut++], fileOut);
            compLen -= 1;
        }
    }

    free(pReadBuffer);
    free(pCompressBuffer);

    return true;
}

/*--------------------------------------------------------------------------*/

