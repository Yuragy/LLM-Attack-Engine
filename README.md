

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

In the web interface are available:
- Configuring and launching attacks.  
- Viewing and filtering logs, graphics.  
- Task scheduling (Scheduler).  
- User management (creation, deletion, role assignment).  
- Data export/import.
