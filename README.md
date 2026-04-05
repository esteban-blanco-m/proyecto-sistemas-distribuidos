# Proyecto: Gestión de Movilidad - Sistemas Distribuidos

## 🏗️ Modelo Arquitectónico
Este diagrama describe la organización de los componentes y el flujo de eventos a través de los nodos.

```mermaid
graph TD
    subgraph PC1 [Nodo 1: Captura]
        S[pc1_sensores.cpp <br/><b>PUB</b>] --> B[pc1_broker.cpp <br/><b>XSUB/XPUB</b>]
    end

    subgraph PC2 [Nodo 2: Procesamiento]
        B --> A[pc2_analitica.cpp <br/><b>SUB</b>]
        A --> R[pc2_replica.cpp <br/><b>Respaldo Local</b>]
    end

    subgraph PC3 [Nodo 3: Persistencia]
        A -- "PUSH / PULL" --> DB[(pc3_db_mongo.cpp <br/><b>MongoDB</b>)]
    end

    %% Estilos para que se vea profesional
    style PC1 fill:#f5f5f5,stroke:#333,stroke-width:2px
    style PC2 fill:#e1f5fe,stroke:#01579b,stroke-width:2px
    style PC3 fill:#e8f5e9,stroke:#2e7d32,stroke-width:2px
