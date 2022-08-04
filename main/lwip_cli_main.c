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

#include "easy_display.h"
#include <u8x8.h>

#define MAX_INPUT_LENGTH    50
#define MAX_OUTPUT_LENGTH   100

#ifndef SOCK_SERVER_PORT
#define SOCK_SERVER_PORT 23
#endif

#define sys_thread_free() vTaskDelete(NULL)

static const char *TAG_TELNET_SRV = "TelnetServer";
static const char *TAG_TELNET_CLI = "TelnetClient";

void main_task(void *ignore) {
    
    // Inicialização

    while(1){
        vTaskDelay(1000);
        // Infinite loop
    }
}
/**
 * @brief xRemotePrint
 * @param pcWriteBuffer  	This is the buffer into which any generated output should be written. For example, if the function is simply going to return the fixed string "Hello World", then the string is written into pcWriteBuffer. Output must always be null terminated.
 * @param xWriteBufferLen  	This is the size of the buffer pointed to by the pcWriteBuffer parameter. Writing more than xWriteBufferLen characters into pcWriteBuffer will cause a buffer overflow.
 * @param pcCommandString  	A pointer to the entire command string. Having access to the entire command string allows the function implementation to extract the command parameters - if there are any. FreeRTOS+CLI provides helper functions that accept the command string and return the command parameters - so explicit string parsing is not required. Examples are provided on this page.
 * @return BaseType_t
*/
BaseType_t xRemotePrint( int8_t *pcWriteBuffer, size_t xWriteBufferLen, const int8_t *pcCommandString){
    // "oi, não sei oq!!"
    if(pcCommandString==NULL){
        // sprintf(pcWriteBuffer,(int8_t*)"Erro!");
        pcWriteBuffer = (int8_t*)"Erro!";
        return pdFALSE;
    }

    // xDisplayPrint //IMPLEMENTAR ESSA PARTE (ideia: Mostrar IP, linha, mensagens (com IP do cliente??))
    // pcWriteBuffer[0]=0;

    unsigned int i=0;
    while(pcCommandString[i]!=0)
    {
        pcWriteBuffer[i]=pcCommandString[i];
        i++;
    }
    pcWriteBuffer[i]=0;
    return pdFALSE;
}

void NewClient( void *pvParameters);
void SocketTelnetServer( void *pvParameters )
{
    int sockfd, newsockfd, clilen;
    struct sockaddr_in serv_addr, cli_addr;
    char buf[64];
    
    //getting current esp32 sta ip
    esp_netif_t* netif = esp_netif_get_handle_from_ifkey("WIFI_STA_DEF");
    esp_netif_ip_info_t ip_info;
    
    esp_netif_get_ip_info(netif, &ip_info);

    if(esp_ip4addr_ntoa(&ip_info.ip, buf, sizeof(buf))==NULL)
    {
        return;
    }
    ESP_ERROR_CHECK_WITHOUT_ABORT(xDisplayClear());
    ESP_ERROR_CHECK_WITHOUT_ABORT(xDisplayWrite("Conecte-se ao IP:"));
    ESP_ERROR_CHECK_WITHOUT_ABORT(xDisplayWrite(buf));

    //creating example command
    CLI_Command_Definition_t xSampleCommand={
        "print",
        "\r\nprint:"
        "\r\n\teste comando escreve o texto no display\r\n",
        xRemotePrint,
        1
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

    nbytes = lwip_read(clientfd, buffer, 128);
    // static const int8_t * pcWelcomeMessage = "Welcome to the FreeRTOS Telnet CLI Server\r\nType Help to view a list of registered commands\r\n";
    static const char * pcWelcomeMessage = "\r\nWelcome to the FreeRTOS Telnet CLI Server\r\nType Help to view a list of registered commands\r\n";
    // (const int8_t *)
    lwip_send(clientfd, pcWelcomeMessage, strlen(pcWelcomeMessage), 0);

    do
    {
        /* This implementation reads a single character at a time.  Wait in the
        Blocked state until a character is received. */
        len = lwip_read(clientfd, &cRxedChar, 1);

        if( cRxedChar == '\r' )
        {
            /* A newline character was received, so the input command string is
            complete and can be processed.  Transmit a line separator, just to
            make the output easier to read. */

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
                    xMoreDataToFollow = FreeRTOS_CLIProcessCommand
                                    (
                                        pcInputString,   /* The command string.*/
                                        pcOutputString,  /* The output buffer. */
                                        MAX_OUTPUT_LENGTH/* The size of the output buffer. */
                                    );

                    /* Write the output generated by the command interpreter to the
                    console. */
                    lwip_send(clientfd,pcOutputString,strlen(pcOutputString),0);

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
                lwip_send(clientfd,pcWelcomeMessage, strlen( pcWelcomeMessage),0);
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
}

void app_main(void)
{
    ESP_ERROR_CHECK( nvs_flash_init() );
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    ESP_ERROR_CHECK(xDisplayInit());

    /* This helper function configures Wi-Fi or Ethernet, as selected in menuconfig.
     * Read "Establishing Wi-Fi or Ethernet Connection" section in
     * examples/protocols/README.md for more information about this function.
     */
    ESP_ERROR_CHECK(example_connect());
    // xTaskCreate(&main_task, "main_task", 10240, NULL, 5, NULL);
    xTaskCreate(&SocketTelnetServer, "TelnetCLITask", 10240, NULL, 0, NULL);
    //main_task(NULL);
}

