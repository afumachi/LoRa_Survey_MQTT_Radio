import sys
import subprocess
import time
import os

# sys.executable usa exatamente o mesmo Python executando o script principal
python_cmd = sys.executable

# Diretório onde este script principal está localizado (raiz do projeto)
base_dir = os.path.dirname(os.path.abspath(__file__))

# Pasta de logs para stdout/stderr de cada processo
log_dir = os.path.join(base_dir, "logs_subprocessos")
os.makedirs(log_dir, exist_ok=True)


def iniciar_processo(nome, caminho_relativo):
    """
    Inicia um subprocesso Python com:
    - cwd = pasta onde o script está (para paths relativos internos funcionarem)
    - stdout/stderr redirecionados para arquivo de log
    """
    script_path = os.path.join(base_dir, caminho_relativo)
    script_dir = os.path.dirname(script_path)
    script_file = os.path.basename(script_path)

    log_path = os.path.join(log_dir, f"{nome}.log")
    log_file = open(log_path, "w", encoding="utf-8")

    processo = subprocess.Popen(
        [python_cmd, script_file],
        cwd=script_dir,
        stdout=log_file,
        stderr=subprocess.STDOUT,
        text=True,
    )
    return processo, log_file, log_path


# 1. Inicia o primeiro processo em segundo plano (N6 - Exibição)
p1, log1, path1 = iniciar_processo("N6", "5_N6_Exibição/N6_Gerencia_SNR_Radio_Vs7.py")

# 2. Inicia o segundo processo em segundo plano (N2_N3 - MQTT)
p2, log2, path2 = iniciar_processo("N2_N3", "2_N2_N3_Python/N2_N3_LSS_MQTT_Vs4.py")

# 3. Inicia o terceiro processo em segundo plano (N5 - Processamento)
p3, log3, path3 = iniciar_processo("N5", "4_N5_Processamento/N5_Gerencia_SNR_Vs3.py")

print("Script principal e subprocessos iniciados com sucesso!")
print(f"Logs sendo gravados em: {log_dir}\n")

# Verificação rápida: dá tempo dos processos falharem logo de cara
# (ex.: erro de import, arquivo não encontrado, porta MQTT indisponível, etc.)
time.sleep(2)

processos = [("N6", p1, path1), ("N2_N3", p2, path2), ("N5", p3, path3)]

algum_falhou = False
for nome, p, log_path in processos:
    if p.poll() is not None:
        algum_falhou = True
        print(f"[ERRO] Processo {nome} encerrou prematuramente (código {p.returncode})")
        print(f"       Veja o log completo em: {log_path}")
        try:
            with open(log_path, "r", encoding="utf-8") as f:
                conteudo = f.read().strip()
                if conteudo:
                    print(f"       Últimas linhas do log:\n{'-'*50}")
                    print("\n".join(conteudo.splitlines()[-15:]))
                    print("-" * 50)
        except Exception:
            pass
    else:
        print(f"[OK] Processo {nome} rodando normalmente (PID {p.pid})")

if algum_falhou:
    print("\nAtenção: um ou mais processos falharam ao iniciar. Verifique os logs acima.")
else:
    print("\nTodos os processos estão de pé. Monitorando... (Ctrl+C para encerrar)")

try:
    while True:
        time.sleep(1)
        # Verificação contínua: avisa se algum processo cair depois de já ter iniciado
        for nome, p, log_path in processos:
            if p.poll() is not None:
                print(f"\n[AVISO] Processo {nome} encerrou inesperadamente (código {p.returncode}). Log: {log_path}")
                # Evita spam: remove da lista de monitoramento após avisar uma vez
                processos = [item for item in processos if item[1] is not p]
except KeyboardInterrupt:
    print("\nEncerrando todos os processos...")
    for nome, p, _ in [("N6", p1, path1), ("N2_N3", p2, path2), ("N5", p3, path3)]:
        if p.poll() is None:
            p.terminate()
    for nome, p, _ in [("N6", p1, path1), ("N2_N3", p2, path2), ("N5", p3, path3)]:
        try:
            p.wait(timeout=5)
        except subprocess.TimeoutExpired:
            print(f"[FORÇANDO] Processo {nome} não respondeu ao terminate(), usando kill()")
            p.kill()
            p.wait()
    for f in (log1, log2, log3):
        f.close()
    print("Processos encerrados com sucesso.")
