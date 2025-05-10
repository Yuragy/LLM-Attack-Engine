Сentralized framework for launching and managing a wide range of password attacks (dictionary, brute force, masks, rules and others), with CLI and web panel, monitoring, task scheduler, cloud integration and ML modules to improve efficiency.

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

### Rule Engine

- Load and manage basic and custom rules (dictionary_rules.txt, brute_force_rules.txt).
- Applying rules to dictionaries: generation of new word combinations, filtering, etc.

### Monitoring and logging

- Logger: several levels (TRACE, DEBUG, INFO, WARNING, ERROR), JSON/XML export support, integration with remote services (AWS CloudWatch, Zabbix, Nagios).
- Monitor: collection of metrics (GPU load, memory, temperature), real-time log analysis, notifications.

### NotificationUtils

- Send notifications via email (SMTP), SMS (Twilio), push (FCM), etc.
- Logging of successful/unsuccessful attempts.
- Integration with Config for parameters (SMTP server, tokens, API keys).

### Scheduler

- Scheduling periodic tasks: database backup, log cleanup, launching test attacks, notifications.
- Complex scheduling templates.
- Managing dependencies between tasks, generating reports.


### User management

- Create/delete accounts, assign roles, activate/deactivate.
- 2FA support (QR codes, TOTP), password recovery.
- Export/import users (CSV), detailed reports.

### CloudIntegration / CloudUtils

- Upload/download files from the cloud (AWS S3, GCS, Azure).
- Asynchronous methods for large files (std::future).
- Encryption/decryption before transfer.
- Caching, logging.

### DataUtils

- Serialization/deserialization of JSON, XML, YAML.
- Data structure validation, filtering, sorting.
- Format conversion for export/import.

### ThreadingUtils / GPUtils

- ThreadingUtils: multithreaded task distribution, prioritized queues, scheduling strategies.
- GPUUtils: interaction with NVML, task allocation to GPUs, collection of load, temperature, memory metrics.

Some parts of the project are intentionally cut out to avoid illegal use. This is a demonstration of the use of ML models in a centralized tool for sophisticated attacks.

## Example of the attack process

1. A user via CLI or web interface launches a dictionary attack against a specific target.
2. AttackEngine receives the command, initializes resources.
3. RuleEngine applies rules to the dictionary, generating a list of candidates for attack.
4. GPUManager distributes candidates among multiple GPUs for parallel verification.
5. Monitor collects execution metrics, Logger records the progress of the attack in logs.
6. User stops/pauses the attack if necessary (stopAttack(), pauseAttack()).
7. Scheduler can periodically launch similar attacks on other targets or back up the base.
8. DBManager stores results, logs, statistics in the database, and generates a report at the end if desired.

## Resources used for work
* [https://www.infosecinstitute.com/resources/hacking/hashcat-tutorial-beginners/] — parsing dictionary, mask, hybrid and brute force attacks with Hashcat
