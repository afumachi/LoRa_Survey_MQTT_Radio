import sys
import subprocess
import time

# sys.executable usa exatamente o mesmo Python executando o script principal
python_cmd = sys.executable



# 1. Inicia o primeiro processo em segundo plano
p1 = subprocess.Popen([python_cmd, "5_N6_Exibição/N6_Gerencia_SNR_Radio_Vs7.py"])

# 2. Inicia o segundo processo em segundo plano
p2 = subprocess.Popen([python_cmd, "2_N2_N3_Python/N2_N3_LSS_MQTT_Vs4.py"])

# 3. Inicia o terceiro processo em segundo plano (corrigida a extensão .py)
p3 = subprocess.Popen([python_cmd, "4_N5_Processamento/N5_Gerencia_SNR_Vs3.py"])

print("Script principal e subprocessos iniciados com sucesso!")

try:
    while True:
        time.sleep(1)

except KeyboardInterrupt:
    print("\nEncerrando todos os processos...")
    p1.terminate()
    p2.terminate()
    p3.terminate()

    p1.wait()
    p2.wait()
    p3.wait()
    print("Processos encerrados com sucesso.")

