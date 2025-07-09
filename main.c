#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdbool.h>
#include "include/formater.h"

// Escribe el carácter especial, copia el campo y verifica si está enfocado
bool campo_listo_para_escribir() {
    system("xdotool type --delay 50 '&'");
    usleep(100 * 1000);

    // Selecciona todo (Ctrl+A) y copia (Ctrl+C)
    system("xdotool key ctrl+a");
    usleep(50 * 1000);
    system("xdotool key ctrl+c");
    usleep(100 * 1000);

    // Lee el portapapeles
    FILE *fp = popen("xclip -o -selection clipboard", "r");
    if (!fp) return false;
    char buffer[256];
    bool found = false;
    if (fgets(buffer, sizeof(buffer), fp)) {
        if (strchr(buffer, '&')) found = true;
    }
    pclose(fp);

    // Borra el campo si estaba listo
    if (found) system("xdotool key BackSpace");
    return found;
}

void tabular_hasta_campo() {
    int intentos = 20; // Evita bucles infinitos
    while (intentos--) {
        if (campo_listo_para_escribir()) return;
        system("xdotool key Tab");
        usleep(100 * 1000);
    }
}

bool starts_with(const char *str, const char *prefix) {
    return strncmp(str, prefix, strlen(prefix)) == 0;
}

int main() {
    char input[256];
    char url[150], user[100], password[100];
    char url_base[200];
    char url_with_login[300];
    char command[600];
    int count = 0;

    if ( formater_ended ==true) {
    
    FILE *file = fopen("salida.txt", "r");
    if (!file) {
        perror("No se pudo abrir el archivo de logins");
        return 1;
    }

    while (fgets(input, sizeof(input), file)) {
        input[strcspn(input, "\n")] = 0;
        if (strlen(input) == 0) continue;

        if (sscanf(input, "%149[^:]:%99[^:]:%99[^\n]", url, user, password) == 3) {
            // Asegura que la URL comience con https://
            if (starts_with(url, "http://") || starts_with(url, "https://")) {
                snprintf(url_base, sizeof(url_base), "%s", url);
            } else {
                snprintf(url_base, sizeof(url_base), "https://%s", url);
            }

            // Solo añade "/login" si NO hay ningún '/' en el url original
            /*if (strchr(url, '/') == NULL) {
                snprintf(url_with_login, sizeof(url_with_login), "%s/login", url_base);
            } else {
                snprintf(url_with_login, sizeof(url_with_login), "%s", url_base);
            }*/

            // Abre la URL en una nueva pestaña de Firefox
            snprintf(command, sizeof(command), "firefox --new-tab \"%s\" &", url_with_login);
            system(command);

            // Espera a que la página web cargue antes de automatizar (ajusta el tiempo según tu conexión)
            sleep(6); // Recomendado: 6-10 segundos para páginas lentas[1][2][4]

            // Busca la ventana de Firefox y la activa
            snprintf(command, sizeof(command),
                "xdotool search --onlyvisible --class Firefox windowactivate --sync");
            system(command);

            // Tabula hasta que el campo usuario esté listo
            tabular_hasta_campo();

            // Escribe el usuario
            snprintf(command, sizeof(command), "xdotool type --delay 50 \"%s\"", user);
            system(command);

            // TAB al campo contraseña
            system("xdotool key Tab");
            usleep(100 * 1000);

            // Tabula hasta que el campo contraseña esté listo
            tabular_hasta_campo();

            // Escribe la contraseña
            snprintf(command, sizeof(command), "xdotool type --delay 50 \"%s\"", password);
            system(command);

            // Presiona ENTER para enviar el formulario
            system("xdotool key Return");

            count++;
            // Ya no es necesario sleep largo aquí, porque el sleep está justo después de abrir la URL
        } else {
            printf("Formato incorrecto en la línea: %s\n", input);
        }
    }

    fclose(file);
    printf("Se han automatizado %d inicios de sesión en Firefox.\n", count);
    return 0;
}
}
