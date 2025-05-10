

### Features
- Various attack methods: dictionary attacks, brute force, masks, rules, etc.
- Dictionary and rule management: support for downloading dictionaries, applying custom and leaked dictionaries, and flexible RuleEngine.
- Web interface: easy-to-use web panel for customizing attacks, viewing logs, and scheduling tasks.
- CLI: powerful command line for fine-grained configuration and automation.
- Monitoring and notifications: real-time tracking of attack progress, sending notifications (email, SMS, push).
- Cloud integration: store and manage data in AWS S3, GCS, and more.
- User management: creation and administration of roles, profiles, 2FA, etc.
- Machine learning: training (MLModelTrainer) and prediction (MLPredictor) modules to improve the effectiveness of attacks.
- Adaptive attacks: change strategy in real time based on target behavior.
- Data recovery: backup and automatic recovery.

### Required Components
1. CMake
2. GCC or Clang (C++) 
3. libraries:  
   - Boost  
   - OpenSSL 
   - NVML  
   - pugiXML, YAML.  
## Usage
Allows you to launch attacks, manage configuration, dictionaries, view logs and more.

- Launch a dictionary attack:  ./password_attack_framework --attack dictionary \ --dictionary path/to/dictionary.txt \. --target target_identifier
      
- View logs:  ./password_attack_framework --view-logs

- Start the web server:  ./password_attack_framework --start-web

Then open http://localhost:8080 in a browser (the port can be specified in the configuration). 

## Configuration
All configuration files are located in the config/ folder:
- config.json - basic parameters (logging, web server ports, dictionary paths, etc.).  
- monitoring_config.json - monitoring module settings (polling frequency, integration with Prometheus/Grafana).  
- db_config.json - database settings (address, login/password, connection parameters).  
- encryption_keys.json - example of secure storage of encryption keys.


In the web interface are available:
- Configuring and launching attacks.  
- Viewing and filtering logs, graphics.  
- Task scheduling (Scheduler).  
- User management (creation, deletion, role assignment).  
- Data export/import.

# Internal Architecture

### Attack Engine
- Functions: logic of different types of attacks (dictionary, brute force, mask, rule-based, etc.).
- Operation: initializes GPU (via GPUManager), loads dictionaries (DictionaryLoader), applies rules (RuleEngine).
- Status: can start/stop/pause/resume, collect metrics, logging.

### Target Interface and Target Factory

- TargetInterface describes the methods that each “target” module must implement: initialization, attack launch, logging, status.
- TargetFactory creates an object of the desired target type (e.g. MetaMaskTarget, ExodusTarget, ElectrumTarget) at the request of AttackEngine.

### Database module

- Connects to the database, executes SQL queries, manages sessions, caching, backups, restores.
- Stores data about users, dictionaries, roles, logs, etc.
- Can encrypt data (AES/OpenSSL), log operations, real-time monitoring.

### Machine Learning

- MLPredictor: load trained models, predictions, accuracy assessment, interpretation.
- MLModelTrainer: training new models, tuning hyperparameters, cross-validation.
