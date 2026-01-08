# C-Sentinel Demo Scripts

Scripts para demostrar C-Sentinel en AIX, optimizados para grabación y generación de GIFs para redes sociales.

## Scripts Disponibles

| Script | Duración | Uso |
|--------|----------|-----|
| `csentinel-demo.sh` | ~3 min | Demo completa con explicaciones |
| `csentinel-gif-demo.sh` | ~45 seg | Demo visual para GIF medio |
| `csentinel-quick-demo.sh` | ~15 seg | Demo ultra-corta para LinkedIn |

## Grabación en AIX

### Opción 1: script + scriptreplay (nativo AIX)

```bash
# Grabar
script -t 2>timing.txt session.txt
./csentinel-quick-demo.sh
exit

# Reproducir
scriptreplay timing.txt session.txt
```

### Opción 2: asciinema (recomendado)

```bash
# Instalar asciinema (requiere Python)
pip3 install asciinema

# Grabar
asciinema rec -c './csentinel-quick-demo.sh' demo.cast

# Reproducir
asciinema play demo.cast
```

## Conversión a GIF

### En Linux/Mac (después de transferir el .cast)

```bash
# Instalar agg (Asciinema GIF Generator)
cargo install --git https://github.com/asciinema/agg

# Convertir a GIF
agg demo.cast demo.gif \
    --cols 80 \
    --rows 24 \
    --speed 1.2 \
    --font-size 14 \
    --theme monokai

# Optimizar tamaño
gifsicle -O3 --colors 128 demo.gif -o demo-optimized.gif
```

### Alternativa: gifcast.com

1. Subir el archivo `.cast` a https://gifcast.com
2. Ajustar configuración (velocidad, tamaño)
3. Descargar GIF

## Tips para LinkedIn

1. **Duración ideal**: 15-30 segundos (autoplay en feed)
2. **Resolución**: 800x600 o menor para carga rápida
3. **Colores**: Alto contraste (tema oscuro funciona bien)
4. **Texto**: Grande y legible en móvil
5. **Call to action**: URL visible al final

## Personalización

Variables de entorno:

```bash
# Pausas más largas (para narración)
DEMO_PAUSE=4 ./csentinel-demo.sh

# Sin colores (para terminales básicas)
NO_COLOR=1 ./csentinel-demo.sh
```

## Contenido de la Demo

### 1. Quick Analysis (`-q`)
- Estado del sistema en un vistazo
- Procesos, memoria, load average
- Detección de problemas (zombies, high FD)

### 2. Network Monitoring (`-n`)
- Puertos TCP/UDP escuchando
- Conexiones establecidas
- **PID attribution** para 70+ servicios conocidos
- Detección de puertos inusuales

### 3. Baseline Learning (`-l`)
- Captura del estado "normal"
- Guardado en ~/.sentinel/baseline.dat

### 4. Drift Detection (`-b`)
- Comparación contra baseline
- Detecta: nuevos listeners, configs modificadas
- Alertas de desviación

### 5. JSON Output (`-j`)
- Salida estructurada para AI/LLM
- Integración con Claude, GPT, etc.
- Automatización y dashboards

## Ejemplo de Post LinkedIn

```
🔍 Monitoring AIX systems just got easier!

C-Sentinel provides:
✅ Quick system health checks
✅ Network monitoring with PID detection
✅ Baseline drift detection
✅ JSON output for AI analysis

Open source. Zero dependencies. One binary.

Install: dnf install csentinel4aix
Repo: aix.librepower.org

#AIX #IBMPower #OpenSource #SysAdmin #DevOps
```

## Licencia

MIT - Parte del proyecto LibrePower
