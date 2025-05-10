document.addEventListener('DOMContentLoaded', function() {
    const filterLogsForm = document.getElementById('filterLogsForm');
    if (filterLogsForm) {
        filterLogsForm.addEventListener('submit', handleFilterLogsForm);
    }

    const exportJsonButton = document.getElementById('exportJson');
    const exportXmlButton = document.getElementById('exportXml');
    const exportCsvButton = document.getElementById('exportCsv');
    if (exportJsonButton) exportJsonButton.addEventListener('click', () => exportLogs('json'));
    if (exportXmlButton) exportXmlButton.addEventListener('click', () => exportLogs('xml'));
    if (exportCsvButton) exportCsvButton.addEventListener('click', () => exportLogs('csv'));

    const loginForm = document.getElementById('loginForm');
    if (loginForm) {
        loginForm.addEventListener('submit', handleLoginForm);
    }

    const logsChartElement = document.getElementById('logsChart');
    if (logsChartElement) {
        initializeLogsChart(logsChartElement);
    }

    const tasksCalendar = document.getElementById('tasksCalendar');
    if (tasksCalendar) {
        initializeTasksCalendar(tasksCalendar);
    }

    checkUpcomingTasks();

    initializeWebSocket();

    fetchDataWithLoading('/api/get_example_data')
        .then(data => {
            console.log('Данные успешно загружены:', data);
        })
        .catch(error => {
            console.error('Ошибка:', error);
        });

    initializeAttackManagement();

    initializeUserManagement();
});

function handleFilterLogsForm(event) {
    event.preventDefault();
    const logLevel = document.getElementById('logLevel').value;
    const logTag = document.getElementById('logTag').value;
    const logDate = document.getElementById('logDate').value;

    fetch('/api/filter_logs', {
        method: 'POST',
        headers: {
            'Content-Type': 'application/json'
        },
        body: JSON.stringify({ level: logLevel, tag: logTag, date: logDate })
    })
    .then(response => response.json())
    .then(data => {
        updateLogEntries(data.logs);
    })
    .catch(error => console.error('Ошибка получения логов:', error));
}

function updateLogEntries(logs) {
    const logEntries = document.getElementById('logEntries');
    if (!logEntries) return;
    logEntries.innerHTML = '';
    logs.forEach(log => {
        const tr = document.createElement('tr');
        tr.innerHTML = `<td>${log.date}</td><td>${log.level}</td><td>${log.tag}</td><td>${log.message}</td>`;
        logEntries.appendChild(tr);
    });
}
function exportLogs(format) {
    fetch(`/api/export_logs?format=${format}`)
    .then(response => response.blob())
    .then(blob => {
        const url = window.URL.createObjectURL(blob);
        const a = document.createElement('a');
        a.style.display = 'none';
        a.href = url;
        a.download = `logs.${format}`;
        document.body.appendChild(a);
        a.click();
        window.URL.revokeObjectURL(url);
    })
    .catch(error => console.error('Ошибка экспорта логов:', error));
}

function handleLoginForm(event) {
    event.preventDefault();
    const username = document.getElementById('username').value;
    const password = document.getElementById('password').value;
    const rememberMe = document.getElementById('rememberMe').checked;
    const totp = document.getElementById('totp').value;
    const captcha = grecaptcha.getResponse();

    fetch('/api/login', {
        method: 'POST',
        headers: {
            'Content-Type': 'application/json'
        },
        body: JSON.stringify({ username, password, rememberMe, totp, captcha })
    })
    .then(response => response.json())
    .then(data => {
        if (data.success) {
            window.location.href = '/dashboard';
        } else {
            displayError(data.message);
            grecaptcha.reset();
        }
    })
    .catch(error => {
        displayError('Произошла ошибка. Пожалуйста, попробуйте еще раз.');
        grecaptcha.reset();
    });
}
function displayError(message) {
    const errorMessage = document.getElementById('errorMessage');
    if (!errorMessage) return;
    errorMessage.style.display = 'block';
    errorMessage.textContent = message;
}
function initializeLogsChart(logsChartElement) {
    const logsData = {
        labels: ["Январь", "Февраль", "Март", "Апрель", "Май", "Июнь"],
        datasets: [{
            label: 'Количество логов',
            data: [50, 60, 70, 80, 90, 100],
            backgroundColor: 'rgba(54, 162, 235, 0.2)',
            borderColor: 'rgba(54, 162, 235, 1)',
            borderWidth: 1
        }]
    };

    new Chart(logsChartElement.getContext('2d'), {
        type: 'line',
        data: logsData,
        options: {
            responsive: true,
            scales: {
                y: {
                    beginAtZero: true
                }
            },
            plugins: {
                legend: {
                    position: 'top',
                },
                title: {
                    display: true,
                    text: 'Количество логов по месяцам'
                }
            }
        }
    });
}
function initializeTasksCalendar(tasksCalendarElement) {
    fetch('/api/scheduled_tasks')
    .then(response => response.json())
    .then(data => {
        const events = data.tasks.map(task => {
            return {
                title: task.name,
                start: task.time,
                description: task.description
            };
        });
        const calendar = new FullCalendar.Calendar(tasksCalendarElement, {
            initialView: 'dayGridMonth',
            events: events,
            eventClick: function(info) {
                alert('Задача: ' + info.event.title + '\nОписание: ' + info.event.extendedProps.description);
            }
        });
        calendar.render();
    })
    .catch(error => console.error('Ошибка загрузки задач:', error));
}
function checkUpcomingTasks() {
    fetch('/api/upcoming_tasks')
    .then(response => response.json())
    .then(data => {
        data.tasks.forEach(task => {
            notifyUser(`У вас запланирована задача "${task.name}" на ${task.time}`);
        });
    })
    .catch(error => console.error('Ошибка проверки предстоящих задач:', error));
}
function notifyUser(message) {
    const notification = new Notification('Напоминание о задаче', {
        body: message,
        icon: '/static/images/notification-icon.png'
    });
}
function initializeWebSocket() {
    const socket = new WebSocket('ws://localhost:8000/ws/updates');
    socket.onmessage = function(event) {
        const data = JSON.parse(event.data);
        if (data.type === 'log_update') {
            updateLogEntries(data.logs);
        }
        if (data.type === 'task_update') {
            updateTaskCalendar(data.tasks);
        }
    };
}
function updateTaskCalendar(tasks) {
    const tasksCalendar = document.getElementById('tasksCalendar');
    if (!tasksCalendar) return;
    const calendar = new FullCalendar.Calendar(tasksCalendar, {
        initialView: 'dayGridMonth',
        events: tasks.map(task => ({
            title: task.name,
            start: task.time,
            description: task.description
        })),
        eventClick: function(info) {
            alert('Задача: ' + info.event.title + '\nОписание: ' + info.event.extendedProps.description);
        }
    });
    calendar.render();
}
async function fetchDataWithLoading(url, options = {}) {
    try {
        showLoadingIndicator();
        const data = await fetchData(url, options);
        return data;
    } finally {
        hideLoadingIndicator();
    }
}
async function fetchData(url, options = {}) {
    const response = await fetch(url, options);
    if (!response.ok) {
        throw new Error(`Ошибка загрузки данных: ${response.statusText}`);
    }
    return response.json();
}
function showLoadingIndicator() {
    const loadingIndicator = document.createElement('div');
    loadingIndicator.id = 'loadingIndicator';
    loadingIndicator.textContent = 'Загрузка...';
    loadingIndicator.style.position = 'fixed';
    loadingIndicator.style.top = '50%';
    loadingIndicator.style.left = '50%';
    loadingIndicator.style.transform = 'translate(-50%, -50%)';
    loadingIndicator.style.zIndex = '1000';
    loadingIndicator.style.padding = '10px 20px';
    loadingIndicator.style.backgroundColor = '#000';
    loadingIndicator.style.color = '#fff';
    document.body.appendChild(loadingIndicator);
}

function hideLoadingIndicator() {
    const loadingIndicator = document.getElementById('loadingIndicator');
    if (loadingIndicator) {
        document.body.removeChild(loadingIndicator);
    }
}

function initializeAttackManagement() {
    const startAttackButton = document.getElementById('startAttack');
    const stopAttackButton = document.getElementById('stopAttack');
    if (startAttackButton) {
        startAttackButton.addEventListener('click', startAttack);
    }
    if (stopAttackButton) {
        stopAttackButton.addEventListener('click', stopAttack);
    }
}
function startAttack() {
    fetch('/api/start_attack', { method: 'POST' })
    .then(response => response.json())
    .then(data => {
        if (data.success) {
            alert('Атака успешно запущена');
        } else {
            alert('Ошибка запуска атаки: ' + data.message);
        }
    })
    .catch(error => console.error('Ошибка запуска атаки:', error));
}
function stopAttack() {
    fetch('/api/stop_attack', { method: 'POST' })
    .then(response => response.json())
    .then(data => {
        if (data.success) {
            alert('Атака успешно остановлена');
        } else {
            alert('Ошибка остановки атаки: ' + data.message);
        }
    })
    .catch(error => console.error('Ошибка остановки атаки:', error));
}
function initializeUserManagement() {
    const addUserForm = document.getElementById('addUserForm');
    const deleteUserButton = document.getElementById('deleteUser');
    if (addUserForm) {
        addUserForm.addEventListener('submit', handleAddUserForm);
    }
    if (deleteUserButton) {
        deleteUserButton.addEventListener('click', deleteUser);
    }
}

function handleAddUserForm(event) {
    event.preventDefault();
    const username = document.getElementById('newUsername').value;
    const password = document.getElementById('newPassword').value;
    const role = document.getElementById('newUserRole').value;

    fetch('/api/add_user', {
        method: 'POST',
        headers: {
            'Content-Type': 'application/json'
        },
        body: JSON.stringify({ username, password, role })
    })
    .then(response => response.json())
    .then(data => {
        if (data.success) {
            alert('Пользователь успешно добавлен');
        } else {
            alert('Ошибка добавления пользователя: ' + data.message);
        }
    })
    .catch(error => console.error('Ошибка добавления пользователя:', error));
}

function deleteUser() {
    const username = document.getElementById('deleteUsername').value;

    fetch('/api/delete_user', {
        method: 'POST',
        headers: {
            'Content-Type': 'application/json'
        },
        body: JSON.stringify({ username })
    })
    .then(response => response.json())
    .then(data => {
        if (data.success) {
            alert('Пользователь успешно удален');
        } else {
            alert('Ошибка удаления пользователя: ' + data.message);
        }
    })
    .catch(error => console.error('Ошибка удаления пользователя:', error));
}




