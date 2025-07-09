#include "include/formater.h"

#define MAX_LINE 1024
#define MAX_EXCEPCIONES 100

typedef struct {
    char dominio[256];
    char directorio[256];
} ExcepcionDirectorio;

typedef struct {
    char dominio[256];
    char nuevo_dominio[256];
} ExcepcionDominio;

void trim(char *str) {
    char *start = str;
    while (*start && isspace((unsigned char)*start)) start++;
    if (start != str) memmove(str, start, strlen(start) + 1);

    char *end = str + strlen(str) - 1;
    while (end >= str && isspace((unsigned char)*end)) *end-- = '\0';
}

char* skip_numbers(const char* line) {
    while (*line && isdigit(*line)) line++;
    while (*line && (*line == ' ' || *line == ':')) line++;
    return (char*)line;
}

char* remove_https(char* line) {
    char* url_start = strstr(line, "https://");
    if (url_start) {
        memmove(url_start, url_start + 8, strlen(url_start + 8) + 1);
    }
    return line;
}

void get_domain_and_rest(const char* line, char* domain, char* rest) {
    int i = 0;
    while (line[i] && line[i] != ' ' && line[i] != ':') {
        domain[i] = line[i];
        i++;
    }
    domain[i] = '\0';

    while (line[i] && (line[i] == ' ' || line[i] == ':')) i++;

    strcpy(rest, line + i);
}

void limpiar_espacios_colon(char* str) {
    char limpio[MAX_LINE];
    int j = 0;
    int i = 0;

    while (str[i]) {
        if (str[i] != ':') {
            limpio[j++] = str[i++];
        } else {
            while (j > 0 && limpio[j-1] == ' ') j--;
            limpio[j++] = ':';
            i++;
            while (str[i] == ' ') i++;
        }
    }
    limpio[j] = '\0';
    strcpy(str, limpio);
}

// Lee las excepciones desde archivo
void leer_excepciones(
    const char* archivo, 
    ExcepcionDirectorio* exc_dir, int* num_exc_dir, 
    ExcepcionDominio* exc_dom, int* num_exc_dom
) {
    FILE *fexc = fopen(archivo, "r");
    char linea[MAX_LINE];

    *num_exc_dir = 0;
    *num_exc_dom = 0;

    if (!fexc) {
        printf("No se pudo abrir el archivo de excepciones.\n");
        return;
    }

    while (fgets(linea, MAX_LINE, fexc)) {
        // Excepción de directorio
        char *p_if = strstr(linea, "if ");
        char *p_pp = strstr(linea, "++");
        char *p_vv = strstr(linea, ">>");

        if (p_if && p_pp) {
            p_if += 3; // Saltar "if "
            char *dominio = p_if;
            char *fin_dom = p_pp - 1;
            while (fin_dom > dominio && isspace((unsigned char)*fin_dom)) fin_dom--;
            *(fin_dom + 1) = '\0';

            char *directorio = p_pp + 2;
            while (*directorio && isspace((unsigned char)*directorio)) directorio++;
            char *fin_dir = directorio + strlen(directorio) - 1;
            while (fin_dir > directorio && (isspace((unsigned char)*fin_dir) || *fin_dir == '\n')) *fin_dir-- = '\0';

            strncpy(exc_dir[*num_exc_dir].dominio, dominio, sizeof(exc_dir[*num_exc_dir].dominio));
            strncpy(exc_dir[*num_exc_dir].directorio, directorio, sizeof(exc_dir[*num_exc_dir].directorio));
            trim(exc_dir[*num_exc_dir].dominio);
            trim(exc_dir[*num_exc_dir].directorio);
            (*num_exc_dir)++;
        }
        // Excepción de dominio
        else if (p_if && p_vv) {
            p_if += 3; // Saltar "if "
            char *dominio = p_if;
            char *fin_dom = p_vv - 1;
            while (fin_dom > dominio && isspace((unsigned char)*fin_dom)) fin_dom--;
            *(fin_dom + 1) = '\0';

            char *nuevo_dom = p_vv + 2;
            while (*nuevo_dom && isspace((unsigned char)*nuevo_dom)) nuevo_dom++;
            char *fin_nuevo = nuevo_dom + strlen(nuevo_dom) - 1;
            while (fin_nuevo > nuevo_dom && (isspace((unsigned char)*fin_nuevo) || *fin_nuevo == '\n')) *fin_nuevo-- = '\0';

            strncpy(exc_dom[*num_exc_dom].dominio, dominio, sizeof(exc_dom[*num_exc_dom].dominio));
            strncpy(exc_dom[*num_exc_dom].nuevo_dominio, nuevo_dom, sizeof(exc_dom[*num_exc_dom].nuevo_dominio));
            trim(exc_dom[*num_exc_dom].dominio);
            trim(exc_dom[*num_exc_dom].nuevo_dominio);
            (*num_exc_dom)++;
        }
    }
    fclose(fexc);
}

// Aplica la excepción de dominio si existe
void aplicar_excepcion_dominio(char* domain, ExcepcionDominio* exc_dom, int num_exc_dom) {
    for (int i = 0; i < num_exc_dom; ++i) {
        if (strcmp(domain, exc_dom[i].dominio) == 0) {
            strcpy(domain, exc_dom[i].nuevo_dominio);
            break;
        }
    }
}

// Devuelve el directorio correspondiente si existe
const char* get_directory(const char* domain, ExcepcionDirectorio* exc_dir, int num_exc_dir) {
    for (int i = 0; i < num_exc_dir; ++i) {
        if (strcmp(domain, exc_dir[i].dominio) == 0)
            return exc_dir[i].directorio;
    }
    return "";
}

int main() {

    bool formater_ended =false;
    FILE *fin = fopen("entrada.txt", "r");
    FILE *fout = fopen("salida.txt", "w");
    char buffer[MAX_LINE];

    ExcepcionDirectorio exc_dir[MAX_EXCEPCIONES];
    ExcepcionDominio exc_dom[MAX_EXCEPCIONES];
    int num_exc_dir = 0, num_exc_dom = 0;

    leer_excepciones("excepciones.txt", exc_dir, &num_exc_dir, exc_dom, &num_exc_dom);

    if (!fin || !fout) {
        printf("Error abriendo archivos.\n");
        return 1;
    }

    while (fgets(buffer, MAX_LINE, fin)) {
        buffer[strcspn(buffer, "\n")] = 0;

        char* line = skip_numbers(buffer);
        remove_https(line);

        char domain[256], rest[MAX_LINE];
        get_domain_and_rest(line, domain, rest);

        trim(domain);
        trim(rest);
        limpiar_espacios_colon(rest);

        // Primero, aplicar excepción de dominio
        aplicar_excepcion_dominio(domain, exc_dom, num_exc_dom);

        // Luego, excepción de directorio
        const char* directory = get_directory(domain, exc_dir, num_exc_dir);

        fprintf(fout, "%s%s:%s\n", domain, directory, rest);
    }

    fclose(fin);
    fclose(fout);

    printf("Proceso completado.\n");
    formater_ended = true;

    return 0;
}
