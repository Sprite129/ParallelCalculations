from locust import HttpUser, task

class StaticWebUser(HttpUser):
    @task
    def load_index(self):
        self.client.get("/index.html")

    @task
    def load_page2(self):
        self.client.get("/second_page.html")

    @task
    def load_404(self):
        self.client.get("/none.html")
