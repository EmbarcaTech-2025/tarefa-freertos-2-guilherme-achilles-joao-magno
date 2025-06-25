#include "lwip/apps/mqtt.h"       // Biblioteca MQTT do lwIP
#include "lwipopts.h"             // Configurações customizadas do lwIP
#include "write_display.h"
#include <string.h>               // Para funções como strlen(), memcpy()
#include <stdio.h>                // Para printf() e outras funções de I/O
#include "criptografias.h"

/* Variável global estática para armazenar a instância do cliente MQTT
 * 'static' limita o escopo deste arquivo */
static mqtt_client_t *client;

/* Callback de conexão MQTT - chamado quando o status da conexão muda
 * Parâmetros:
 *   - client: instância do cliente MQTT
 *   - arg: argumento opcional (não usado aqui)
 *   - status: resultado da tentativa de conexão */
static void mqtt_connection_cb(mqtt_client_t *client, void *arg, mqtt_connection_status_t status) {
    if (status == MQTT_CONNECT_ACCEPTED) {
        printf("Conectado ao broker MQTT com sucesso!\n");
    } else {
        printf("Falha ao conectar ao broker, código: %d\n", status);
    }
}

/* Função para configurar e iniciar a conexão MQTT
 * Parâmetros:
 *   - client_id: identificador único para este cliente
 *   - broker_ip: endereço IP do broker como string (ex: "192.168.1.1")
 *   - user: nome de usuário para autenticação (pode ser NULL)
 *   - pass: senha para autenticação (pode ser NULL) */
void mqtt_setup(const char *client_id, const char *broker_ip, const char *user, const char *pass) {
    ip_addr_t broker_addr;  // Estrutura para armazenar o IP do broker
    
    // Converte o IP de string para formato numérico
    if (!ip4addr_aton(broker_ip, &broker_addr)) {
        printf("Erro no IP\n");
        return;
    }else {
        printf("IP do broker: %s\n", broker_ip);
    }

    // Cria uma nova instância do cliente MQTT
    client = mqtt_client_new();
    if (client == NULL) {
        printf("Falha ao criar o cliente MQTT\n");
        return;
    }else {
        printf("Cliente MQTT criado com sucesso!\n");
    }

    // Configura as informações de conexão do cliente
    struct mqtt_connect_client_info_t ci = {
        .client_id = client_id,  // ID do cliente
        .client_user = user,     // Usuário (opcional)
        .client_pass = pass      // Senha (opcional)
    };

    // Inicia a conexão com o broker
    // Parâmetros:
    //   - client: instância do cliente
    //   - &broker_addr: endereço do broker
    //   - 1883: porta padrão MQTT
    //   - mqtt_connection_cb: callback de status
    //   - NULL: argumento opcional para o callback
    //   - &ci: informações de conexão
    mqtt_client_connect(client, &broker_addr, 1883, mqtt_connection_cb, NULL, &ci);
}

/* Callback de confirmação de publicação
 * Chamado quando o broker confirma recebimento da mensagem (para QoS > 0)
 * Parâmetros:
 *   - arg: argumento opcional
 *   - result: código de resultado da operação */
static void mqtt_pub_request_cb(void *arg, err_t result) {
    if (result == ERR_OK) {
        printf("Publicação MQTT enviada com sucesso!\n");
    } else {
        printf("Erro ao publicar via MQTT: %d\n", result);
    }
}

/* Função para publicar dados em um tópico MQTT
 * Parâmetros:
 *   - topic: nome do tópico (ex: "sensor/temperatura")
 *   - data: payload da mensagem (bytes)
 *   - len: tamanho do payload */
void mqtt_comm_publish(const char *topic, const uint8_t *data, size_t len) {
    // Envia a mensagem MQTT
    err_t status = mqtt_publish(
        client,              // Instância do cliente
        topic,               // Tópico de publicação
        data,                // Dados a serem enviados
        len,                 // Tamanho dos dados
        0,                   // QoS 0 (nenhuma confirmação)
        0,                   // Não reter mensagem
        mqtt_pub_request_cb, // Callback de confirmação
        NULL                 // Argumento para o callback
    );

    if (status != ERR_OK) {
        printf("mqtt_publish falhou ao ser enviada: %d\n", status);
    }   
}

static void mqtt_incoming_publish_cb(void *arg, const char *topic, u32_t tot_len) 
{
    // Validações básicas
    if (topic == NULL) {
        printf("ERRO MQTT: Tópico NULL\n");
        return;
    }
    
    if (strlen(topic) == 0) {
        printf("ERRO MQTT: Tópico vazio\n");
        return;
    }
    
    // Log normal se tudo OK
    printf("Mensagem recebida no tópico: %s (%lu bytes)\n", topic, (unsigned long)tot_len);
}

static uint32_t ultima_timestamp_recebida = 0; // Timestamp da última mensagem válida

static void mqtt_incoming_data_cb(void *arg, const u8_t *data, u16_t len, u8_t flags) 
{
    char msg[128];

    // Copia os dados recebidos e adiciona terminador de string
    memcpy(msg, data, len);
    msg[len] = '\0';

    printf("Mensagem criptografada recebida: %s\n", msg);

    // 🔐 Descriptografa com XOR
    uint8_t chave = 42;
    for (int i = 0; i < len; i++) {
        msg[i] = msg[i] ^ chave;
    }

    printf("Dados descriptografados: %s\n", msg);

    // 🛠️ Faz o parse do JSON
    uint32_t nova_timestamp;
    float valor;
    if (sscanf(msg, "{\"valor\":%f,\"ts\":%lu}", &valor, &nova_timestamp) != 2) {
        printf("Erro no parse da mensagem!\n");
        return;
    }

    // 🔍 Verifica se é uma mensagem nova ou replay
    if (nova_timestamp > ultima_timestamp_recebida) {
        ultima_timestamp_recebida = nova_timestamp;

        printf("Nova leitura: %.2f (ts: %lu)\n", valor, nova_timestamp);

        // --> Atualiza display com valor novo
        char text_valor[32];
        snprintf(text_valor, sizeof(text_valor), "%.2f", valor);

        char *text[] = { text_valor };
        char *text_2[] = { "Recebendo:" };

        clear_display();
        write_string(text_2, 1, 2, 1);
        write_string(text, 32, 32, 2);
        show_display();

    } else {
        printf("Replay detectado (ts: %lu <= %lu)\n", nova_timestamp, ultima_timestamp_recebida);

        // --> Mostra no display a mensagem de Replay com o timestamp anterior
        char text_replay[32];
        snprintf(text_replay, sizeof(text_replay), "Replay ts:%lu", ultima_timestamp_recebida);

        char *text[] = { text_replay };
        char *text_2[] = { "Dado antigo" };

        clear_display();
        write_string(text_2, 1, 2, 1);
        write_string(text, 10, 32, 1);
        show_display();
    }
}



void mqtt_comm_subscribe(const char *topic) {
    err_t status;

    // Define os callbacks de chegada de mensagens
    mqtt_set_inpub_callback(client, mqtt_incoming_publish_cb, mqtt_incoming_data_cb, NULL);

    // Faz o subscribe no tópico desejado
    status = mqtt_subscribe(client, topic, 0, NULL, NULL);

    if (status == ERR_OK) {
        printf("Inscrição no tópico '%s' feita com sucesso!\n", topic);
    } else {
        printf("Erro ao fazer subscribe no tópico '%s': %d\n", topic, status);
    }
}

