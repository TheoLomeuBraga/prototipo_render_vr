#pragma once

#include "openxr/openxr.h"
#include "openxr/openxr_platform.h"

// Função auxiliar para verificar erros do OpenXR
void checkOpenXRError(XrResult result, const char* message) {
    if (result != XR_SUCCESS) {
        std::cerr << "Erro " << result << " ao " << message << std::endl;
        exit(-1);
    }
}

XrInstanceCreateInfo createInfo = { XR_TYPE_SESSION_CREATE_INFO };
XrSystemGetInfo systemGetInfo = {};
XrSessionCreateInfo sessionCreateInfo = { XR_TYPE_SESSION_CREATE_INFO };
XrSessionBeginInfo beginInfo = {};

// Inicialização do OpenXR
XrInstance instance;
XrSystemId systemId;
XrSession session;
XrResult result;


//strcpy(createInfo.applicationInfo.applicationName, "prototipo render vr");

// Função para criar uma instância do OpenXR
XrInstance create_openxr_instance() {

    // Definir as configurações para a instância do OpenXR
    createInfo.type = XR_TYPE_INSTANCE_CREATE_INFO;
    createInfo.applicationInfo.applicationVersion = 1;
    createInfo.applicationInfo.engineVersion = 1;
    createInfo.applicationInfo.apiVersion = XR_CURRENT_API_VERSION;

    // Definir o nome da aplicação
    strcpy(createInfo.applicationInfo.applicationName, "prototipo render vr");

    createInfo.enabledApiLayerCount = 0;
    createInfo.enabledApiLayerNames = nullptr;
    createInfo.enabledExtensionCount = 0;
    createInfo.enabledExtensionNames = nullptr;

    // Criar a instância do OpenXR
    result = xrCreateInstance(&createInfo, &instance);
    checkOpenXRError(result, "criar uma instância do OpenXR");

    return instance;
}

void start_openxr() {

    create_openxr_instance();

    // Selecionar o sistema OpenXR (provavelmente VR)
    systemGetInfo.type = XR_TYPE_SYSTEM_GET_INFO;
    systemGetInfo.next = nullptr;
    result = xrGetSystem(instance, &systemGetInfo, &systemId);
    checkOpenXRError(result, "selecionar o sistema OpenXR");

    // Criar uma sessão OpenXR
    sessionCreateInfo.next = nullptr;
    sessionCreateInfo.systemId = systemId;
    systemGetInfo.formFactor = XR_FORM_FACTOR_HEAD_MOUNTED_DISPLAY;
    result = xrCreateSession(instance, &sessionCreateInfo, &session);
    checkOpenXRError(result, "criar uma sessão OpenXR");

    // Configurar e configurar o ambiente de renderização

    // Configurar hand tracking (se necessário)

    // Iniciar a sessão OpenXR
    beginInfo.type = XR_TYPE_SESSION_BEGIN_INFO;
    beginInfo.next = nullptr;
    result = xrBeginSession(session, &beginInfo);
    checkOpenXRError(result, "iniciar a sessão OpenXR");

    // Outras inicializações e configurações podem ser feitas aqui

    // A partir daqui, a sessão OpenXR está ativa e pronta para uso
}




void update_openxr() {
    // Estrutura para armazenar os eventos de OpenXR
    XrEventDataBuffer eventDataBuffer = {};
    eventDataBuffer.type = XR_TYPE_EVENT_DATA_BUFFER;
    eventDataBuffer.next = nullptr;

    // Loop de eventos OpenXR
    while (true) {
        // Pegar o próximo evento OpenXR
        XrResult result = xrPollEvent(instance, &eventDataBuffer);
        if (result != XR_SUCCESS) {
            // Tratar erro
            break;
        }

        // Processar o evento OpenXR, se necessário
        switch (eventDataBuffer.type) {
            // Processar diferentes tipos de eventos OpenXR aqui, se necessário
            // Por exemplo, tratar eventos de entrada do controlador, eventos de sessão, etc.
            default:
                break;
        }

        // Verificar se há mais eventos a serem processados
        if (result == XR_EVENT_UNAVAILABLE) {
            // Não há mais eventos
            break;
        }
    }

    // Atualizar o estado da sessão OpenXR
    XrFrameState frameState = { XR_TYPE_FRAME_STATE };
    xrWaitFrame(session, nullptr, &frameState);

    // Outras atualizações do estado da sessão OpenXR podem ser feitas aqui
}