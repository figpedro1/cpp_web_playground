import { createRouter, createWebHistory, type RouteRecordRaw } from 'vue-router';
import HomeView from '../views/HomeView.vue';

const routes: Array<RouteRecordRaw> = [
    {
        path: '/',
        name: 'home',
        component: HomeView
    },
    {
        path: '/teste',
        name: 'teste',
        component: () => import('../views/TestView.vue')
    }
];

const router = createRouter({
    history: createWebHistory(),
    routes
});

export default router