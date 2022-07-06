/* HTTPS GET Example using plain mbedTLS sockets
 *
 * Contacts the howsmyssl.com API via TLS v1.2 and reads a JSON
 * response.
 *
 * Adapted from the ssl_client1 example in mbedtls.
 *
 * Original Copyright (C) 2006-2016, ARM Limited, All Rights Reserved, Apache 2.0 License.
 * Additions Copyright (C) Copyright 2015-2016 Espressif Systems (Shanghai) PTE LTD, Apache 2.0 License.
 *
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#include <string.h>
#include <stdlib.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_log.h"
#include "esp_system.h"
#include "nvs_flash.h"
#include "protocol_examples_common.h"
#include "esp_netif.h"

#include "lwip/err.h"
#include "lwip/sockets.h"
#include "lwip/sys.h"
#include "lwip/netdb.h"
#include "lwip/dns.h"

#include "esp_tls.h"
#include "esp_crt_bundle.h"

#include <stdio.h>

#include "FreeRTOS_CLI.h"

#define MAX_INPUT_LENGTH    50
#define MAX_OUTPUT_LENGTH   100

#ifndef SOCK_SERVER_PORT
#define SOCK_SERVER_PORT 23
#endif

#define sys_thread_free() vTaskDelete(NULL)

// DELETE
// #include "cJSON.h"

// DELETE
// #include "u8g2_esp32_hal.h"
// #include <u8g2.h>

// DELETE
// // SDA
// #define PIN_SDA 5
// // SCL
// #define PIN_SCL 4

// DELETE
// /* Constants that aren't configurable in menuconfig */
// #define WEB_SERVER "www.commodities-api.com"
// #define WEB_PORT "443"
// #define API_KEY "nnzyuh7xyarsdi8646ebicmw86mlv95n0d2o5weuemz6b62lb0zlde7269xz"
// #define WEB_URL "https://www.commodities-api.com/api/latest?access_key="API_KEY"&base=BRL&symbols=CORN,SOYBEAN,WHEAT"

static const char *TAG_TELNET_SRV = "TelnetServer";
static const char *TAG_TELNET_CLI = "TelnetClient";

// DELETE
// static const char REQUEST[] = "GET " WEB_URL " HTTP/1.1\r\n"
//                              "Host: "WEB_SERVER"\r\n"
//                              "User-Agent: esp-idf/1.0 esp32\r\n"
//                              "\r\n";

/* Root cert for howsmyssl.com, taken from server_root_cert.pem

   The PEM file was extracted from the output of this command:
   openssl s_client -showcerts -connect www.howsmyssl.com:443 </dev/null

   The CA root cert is the last cert given in the chain of certs.

   To embed it in the app binary, the PEM file is named
   in the component.mk COMPONENT_EMBED_TXTFILES variable.
*/

// DELETE
// extern const uint8_t server_root_cert_pem_start[] asm("_binary_server_root_cert_pem_start");
// extern const uint8_t server_root_cert_pem_end[]   asm("_binary_server_root_cert_pem_end");

void main_task(void *ignore) {
    
    // Inicialização

    while(1){
        vTaskDelay(1000);
        // Infinite loop
    }
}

void NewClient( void *pvParameters);
void SocketTelnetServer( void *pvParameters )
{
    int sockfd, newsockfd, clilen;
    struct sockaddr_in serv_addr, cli_addr;

    CLI_Command_Definition_t xSampleCommand={
        "sample_command",
        "\r\nsample_command:"
        "\r\n\teste é um exemplo de descrição de comando\r\n",
        NULL,
        0
    };
    FreeRTOS_CLIRegisterCommand(&xSampleCommand);

    LWIP_UNUSED_ARG(pvParameters);

    sockfd = lwip_socket(AF_INET, SOCK_STREAM, 0);

    if(sockfd < 0) {
        while(1){
            vTaskDelay(1000);
        }
    }

    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_len = sizeof(serv_addr);
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = PP_HTONS(SOCK_SERVER_PORT);
    serv_addr.sin_addr.s_addr = INADDR_ANY;

    if(lwip_bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0)
    {
        lwip_close(sockfd);
        while(1){
            vTaskDelay(1000);
        }
    }

    if(lwip_listen(sockfd,5) != 0){
        lwip_close(sockfd);
        while(1){
            vTaskDelay(1000);
        }
    }

    clilen = sizeof(cli_addr);
    ESP_LOGI(TAG_TELNET_SRV,"Listening for new connections");
    while(1){
        newsockfd = lwip_accept(sockfd, (struct sockaddr *) &cli_addr, (socklen_t *)&clilen);

        if(newsockfd>0){
            if(sys_thread_new("NewClient", NewClient, (void*) &newsockfd, 4096, 5)== NULL){
                lwip_close(newsockfd);
            }
        }
    }
}

void NewClient( void *pvParameters){
    ESP_LOGI(TAG_TELNET_SRV, "New client connected! Started Telnet CLI instance");
    char buffer[128];
    int i, nbytes;
    int len=0;
    // uint8_t data[32];
    int clientfd = *((int*) pvParameters);
    //  Peripheral_Descriptor_t xConsole;
    int8_t cRxedChar, cInputIndex = 0;
    BaseType_t xMoreDataToFollow;
    /* The input and output buffers are declared static to keep them off the stack. */
    // static int8_t pcOutputString[ MAX_OUTPUT_LENGTH ], pcInputString[ MAX_INPUT_LENGTH ];
    static char pcOutputString[ MAX_OUTPUT_LENGTH ], pcInputString[ MAX_INPUT_LENGTH ];
    // CLI_Command_Definition_t xSampleCommand={
    //         "comando",
    //         "\r\ncomando:"
    //         "\r\n\teste é o help do comando, escreva-o com sabedoria",
    //         NULL,
    //         0
    // };
    // FreeRTOS_CLIRegisterCommand(&xSampleCommand);
    nbytes = lwip_read(clientfd, buffer, 128);
    // static const int8_t * pcWelcomeMessage = "Welcome to the FreeRTOS Telnet CLI Server\r\nType Help to view a list of registered commands\r\n";
    static const char * pcWelcomeMessage = "\r\nWelcome to the FreeRTOS Telnet CLI Server\r\nType Help to view a list of registered commands\r\n";
    // (const int8_t *)
    lwip_send(clientfd, pcWelcomeMessage, strlen(pcWelcomeMessage), 0);

    do
    {
        /* This implementation reads a single character at a time.  Wait in the
        Blocked state until a character is received. */
        // len = lwip_recv(clientfd, buffer, sizeof(buffer),0);
        len = lwip_read(clientfd, &cRxedChar, 1);
        // cRxedChar = buffer[0];
        // len = read_usb_cdc(&cRxedChar, 1, portMAX_DELAY);

        if( cRxedChar == '\r' )
        {
            /* A newline character was received, so the input command string is
            complete and can be processed.  Transmit a line separator, just to
            make the output easier to read. */
            // lwip_send(clientfd,"\r\n",2,0);
            // CDC_Transmit_HS("\r\n",2);

            /* The command interpreter is called repeatedly until it returns
            pdFALSE.  See the "Implementing a command" documentation for an
            exaplanation of why this is. */
            if(strlen(pcInputString)>0)
            {
                do
                {
                    /* Send the command string to the command interpreter.  Any
                    output generated by the command interpreter will be placed in the
                    pcOutputString buffer. */
                    // xMoreDataToFollow = FreeRTOS_CLIProcessCommand
                    //                 (
                    //                     (const char *const)pcInputString,   /* The command string.*/
                    //                     (char*)pcOutputString,  /* The output buffer. */
                    //                     MAX_OUTPUT_LENGTH/* The size of the output buffer. */
                    //                 );
                    xMoreDataToFollow = FreeRTOS_CLIProcessCommand
                                    (
                                        pcInputString,   /* The command string.*/
                                        pcOutputString,  /* The output buffer. */
                                        MAX_OUTPUT_LENGTH/* The size of the output buffer. */
                                    );

                    /* Write the output generated by the command interpreter to the
                    console. */
                    lwip_send(clientfd,pcOutputString,strlen(pcOutputString),0);
                    // lwip_send(clientfd,pcOutputString,strlen((const char*)pcOutputString),0);
                    // CDC_Transmit_HS(pcOutputString, strlen( pcOutputString ));

                } while( xMoreDataToFollow != pdFALSE );

                /* All the strings generated by the input command have been sent.
                Processing of the command is complete.  Clear the input string ready
                to receive the next command. */
                cInputIndex = 0;
                memset( pcInputString, 0x00, MAX_INPUT_LENGTH );
            }
        }
        else
        {
            /* The if() clause performs the processing after a newline character
            is received.  This else clause performs the processing if any other
            character is received. */

            // lwip_send(clientfd,&cRxedChar,1,0);
            // CDC_Transmit_HS( &cRxedChar, 1);
            if( cRxedChar == '\n' )
            {
                /* Ignore carriage returns. */
            }
            else if( cRxedChar == '\b' )
            {
                /* Backspace was pressed.  Erase the last character in the input
                buffer - if there are any. */
                if( cInputIndex > 0 )
                {
                    cInputIndex--;
                    pcInputString[ cInputIndex ] = '\0';
                }
            }
            else if( cRxedChar == '\0')
            {
                // lwip_send(clientfd,pcWelcomeMessage, strlen( (const char*)pcWelcomeMessage),0);
                lwip_send(clientfd,pcWelcomeMessage, strlen( pcWelcomeMessage),0);
                // CDC_Transmit_HS(pcWelcomeMessage, strlen( pcWelcomeMessage));
            }
            else
            {
                /* A character was entered.  It was not a new line, backspace
                or carriage return, so it is accepted as part of the input and
                placed into the input buffer.  When a n is entered the complete
                string will be passed to the command interpreter. */
                if( cInputIndex < MAX_INPUT_LENGTH )
                {
                    pcInputString[ cInputIndex ] = cRxedChar;
                    cInputIndex++;
                }
            }
        }
    }while(len>0); // enquanto o cliente manter a conexão aberta
    ESP_LOGI(TAG_TELNET_CLI,"Detected client disconnect! Closing thread");
    lwip_close(clientfd);
    sys_thread_free(); // excluindo a tarefa

	// len = lwip_read(clientfd, buffer, 128);
    // read_usb_cdc(buffer,128,portMAX_DELAY);
}

void app_main(void)
{
    ESP_ERROR_CHECK( nvs_flash_init() );
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());

    /* This helper function configures Wi-Fi or Ethernet, as selected in menuconfig.
     * Read "Establishing Wi-Fi or Ethernet Connection" section in
     * examples/protocols/README.md for more information about this function.
     */
    ESP_ERROR_CHECK(example_connect());
    xTaskCreate(&main_task, "main_task", 10240, NULL, 5, NULL);
    xTaskCreate(&SocketTelnetServer, "TelnetCLITask", 10240, NULL, 0, NULL);
    //main_task(NULL);
}

